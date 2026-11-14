# Garbage Collection Improvements & Root Set Analysis

This document outlines the architectural gaps in JVMX2's garbage collection subsystem (specifically `CheneyGarbageCollector` and `ThreadManager`) regarding Java SE 7 JVM specification compliance.

---

## Incomplete Root Set Scanning

According to JVMS §2.5.3, the root set of a JVM garbage collector must include all active thread operand stacks, local variable tables, static variables, JNI references, and runtime structures. Below are the specific files, function implementations, and line numbers where these root scanning gaps occur in JVMX2:

### 1. Static Fields Across Multiple Class Loaders
- **File**: `JVMX2/ThreadManager.cpp`
- **Lines**: 184–202 (`ThreadManager::GetRoots()`)
- **Issue**: 
  ```cpp
  // We must only get the static objects once.
  if ( nullptr == element.second.m_pThread )
  {
    std::vector<boost::intrusive_ptr<IJavaVariableType>> staticVariables = element.second.m_pVMState->GetStaticObjectsAndArrays();
    allRoots.insert( allRoots.cend(), staticVariables.cbegin(), staticVariables.cend() );
  }
  ```
  Static fields are only retrieved from the primary/main thread's state. If secondary threads or custom class loaders initialize classes with static fields in separate `BasicClassLibrary` instances, those static fields are omitted from root set tracing.
- **Improvement**: Aggregate static fields across all registered `BasicClassLibrary` or class loader instances rather than restricting it to a single thread state.

### 2. Unscanned Constant Pools
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 299–318 (`CheneyGarbageCollector::GetJavaLangClasses` / `GetStringPoolRoots`)
- **Issue**: The GC collects roots from the string pool and registered Java language classes, but does not iterate through the constant pools of all loaded classes (`JavaClass::GetConstantPool()`). Constant pool entries can hold direct object or class references that are live even if not currently on an operand stack.
- **Improvement**: Add a root scanning pass over all active class constant pools in loaded class libraries.

### 3. Native Allocation Heuristic (`m_RecentAllocations`)
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 410–418 (`CheneyGarbageCollector::AddRecentAllocation`) and lines 170–190 (`Collect`)
- **Issue**: 
  ```cpp
  // Make sure that we keep the last c_MaxRecentAllocations recent objects.
  for ( auto it = m_RecentAllocations.begin(); it != m_RecentAllocations.end(); ++ it )
  {
    unqiueRoots.insert( *it );
  }
  ```
  Instead of tracking references explicitly from native/C++ caller contexts, JVMX2 relies on a heuristic cache of the last N allocated objects. This risks premature collection if an object falls out of the recent allocation window while still referenced by internal native code.
- **Improvement**: Replace the recent allocation ring-buffer heuristic with formal JNI handle tracking and explicit root pinning for C++ call frames.

### 4. Finalization Queue Reachability
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 363–391 (`CheneyGarbageCollector::RunFinalisersForOldObjects`)
- **Issue**: Finalizers are executed during or after collection, but objects pending finalization are not formally marked as roots during the initial Cheney copying phase. This can cause objects awaiting `finalize()` execution to have their internal reference fields collected prematurely.
- **Improvement**: Maintain a dedicated finalizer root queue that keeps un-finalized objects alive until their `finalize()` method has successfully run.

### 5. JNI Local Stack Frame Tracking
- **File**: `JVMX2/BasicVirtualMachineState.cpp`
- **Lines**: 2070–2092 (`BasicVirtualMachineState::GetGCRoots()`)
- **Issue**: While `m_LocalReferenceFrames` are scanned, temporary `boost::intrusive_ptr<ObjectReference>` instances created on the C++ stack inside JNI native functions (`JavaNativeInterface.cpp`) are not automatically registered to the current thread's local reference frame, leaving them vulnerable to collection unless explicitly pushed.
- **Improvement**: Automate JNI frame guard wrappers in `JavaNativeInterface.cpp` to register all native return values and temporary arguments.

---

## Thread Synchronization & Safe Points

A spec-compliant garbage collector (especially a copying collector like Cheney's algorithm which relocates objects in memory and updates pointers) requires strict thread synchronization. Below are the specific files, function implementations, and line numbers where thread synchronization and safe point shortcomings occur in JVMX2:

### 1. Unsafe Native Execution Exemption
- **File**: `JVMX2/ThreadManager.cpp`
- **Lines**: 134–163 (`ThreadManager::WaitForThreadsToPause()`)
- **Issue**: 
  ```cpp
  for ( auto element : m_JavaThreads )
  {
    if ( element.first != boost::this_thread::get_id() && !element.second.m_pVMState->IsPaused() )
    {
      if ( !element.second.m_pVMState->IsExecutingNative() )
      {
        allPaused = false;
      }
    }
  }
  ```
  When attempting to pause threads for garbage collection, `WaitForThreadsToPause` actively **ignores** threads that are executing native code (`IsExecutingNative() == true`). 
- **Impact**: If a thread is running native JNI code while Cheney GC is compacting the heap and updating object pointers (`CheneyGarbageCollector::UpdatePointers()`), that native thread can concurrently read/write stale object references or corrupt memory, leading to severe race conditions and crashes.
- **Improvement**: Require threads in native execution to transition through a safepoint or block if they attempt to interact with the managed heap during a garbage collection cycle.

### 2. Non-Cooperative / Timed-Out Collection Deferral
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 146–163 (`CheneyGarbageCollector::Collect()`)
- **Issue**: 
  ```cpp
  m_pThreadManager->PauseAllThreads();
  if ( !m_pThreadManager->WaitForThreadsToPause() )
  {
    #if defined(_DEBUG)
    pLogger->LogDebug( "Could not pause all threads, deferring Garbage Collection..." );
    #endif
    m_IsCollecting = false;
    m_pThreadManager->ResumeAllThreads();
    return;
  }
  ```
  If threads fail to pause within the timeout window (`c_MillisecondsToWaitForAllThreadsdToPause`), the GC simply aborts collection and resumes all threads. 
- **Impact**: This creates a non-cooperative fallback mechanism where memory pressure can build up indefinitely if threads take too long at non-safepoint locations, eventually leading to uncontrolled heap exhaustion (`OutOfMemoryError` failures).
- **Improvement**: Implement cooperative safe points (polling checks in interpreter loops and allocation paths) ensuring deterministic thread suspension without arbitrary timeouts.

---

## Object Finalization (§12.6)

According to Java SE 7 JVM Specification (§12.6), object finalization requires a distinct lifecycle phase where unreachable objects with non-trivial `finalize()` methods are queued and executed prior to memory reclamation. Below are the specific files, function implementations, and line numbers where JVMX2 falls short of §12.6 compliance:

### 1. Synchronous Finalization Execution Inside Stop-The-World GC Phase
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 74–99 (`CheneyGarbageCollector::RunAllFinalizers`) and lines 363–391 (`CheneyGarbageCollector::RunFinalisersForOldObjects`)
- **Issue**: 
  ```cpp
  std::shared_ptr<MethodInfo> pMethodInfo = pVMState->ResolveMethod( pObjectToFinalize->GetContainedObject()->GetClass().get(), c_FinalizeMethodName, c_FinalizeMethodType );
  if ( nullptr != pMethodInfo )
  {
    pVMState->PushOperand( pObjectToFinalize );
    pVMState->ExecuteMethod( *pMethodInfo->GetClass()->GetName(), c_FinalizeMethodName, c_FinalizeMethodType, pMethodInfo );
  }
  ```
  Finalizer methods (`finalize()`) are executed **synchronously inside the garbage collection routine** while all threads are paused (`m_pThreadManager->PauseAllThreads()`).
- **Impact**: Executing arbitrary user bytecode (which can allocate objects, acquire locks, or trigger nested GC cycles) directly inside the GC's collection critical section violates Java semantics and risks deadlocks or heap corruption.
- **Improvement**: Decouple finalization by collecting un-finalized unreachable objects into a pending finalization queue (`F-queue`) during GC, and execute them asynchronously via a dedicated finalizer thread outside the stop-the-world pause.

### 2. Lack of Finalization State Tracking & Resurrection Safeguards
- **File**: `JVMX2/CheneyGarbageCollector.cpp`
- **Lines**: 363–391 (`CheneyGarbageCollector::RunFinalisersForOldObjects`)
- **Issue**: 
  ```cpp
  for ( auto pI = pObjectRegistry->GetFirst(); pObjectRegistry->HasMore( pI ); pI = pObjectRegistry->GetNext( pI ) )
  {
    if ( !pObjectRegistry->HasBeenUpdatedAt( pI ) )
    {
      // Executes finalize() immediately on un-updated objects
    }
  }
  ```
  JVMX2 checks whether objects have been updated during the GC cycle via `HasBeenUpdatedAt` to trigger finalizers, but does not maintain per-object finalization flags (such as whether `finalize()` has already been run or whether an object was resurrected).
- **Impact**: Objects that resurrect themselves during `finalize()` (e.g. by assigning `this` to a global/static variable) are not correctly preserved across GC cycles or protected from multiple finalization calls, violating JVMS §12.6.1.
- **Improvement**: Track finalization status per object instance (e.g., Unfinalized -> Finalizable -> Finalized) and ensure each object's `finalize()` method executes at most once.

---

## Safepoint Implementation Design Pattern (C++11)

To resolve native execution exemption and ensure threads safely synchronize during garbage collection, the following pattern can be implemented:

1. **Thread State Tracking**: Track whether threads are running Java bytecode, executing native JNI code, or parked at a safepoint.
2. **RAII Native Transition Guard**: Wrap JNI native calls in an RAII guard (`ThreadNativeTransition`) that marks the thread as `RunningNative` upon entry and triggers a safepoint check (`TransitionBackFromNative`) upon return.
3. **Safepoint Request Flag & Condition Variable**: Use an atomic flag (`g_SafepointRequested`) and condition variable (`g_SafepointCV`) so threads running native code block upon attempting to return to managed execution or touch the heap while a GC cycle is active.


In a managed runtime (like Java’s HotSpot JVM, the .NET CLR, or Go), a **Stop-The-World (STW) Garbage Collector** needs to freeze all application threads (called **mutator threads**) to safely inspect, move, and clean up heap objects.

Because a thread cannot simply be yanked out of execution mid-CPU-instruction without leaving memory or registers corrupted, the runtime uses **Safepoints**. 

The coordination between the GC thread and the mutator threads to enter and exit a safepoint is a classic, textbook implementation of **condition variables**.

---

### The Problem: The Safepoint Rendezvous

1. **Why Safepoints?** A thread can only pause where the compiler knows *exactly* which registers and stack locations hold object references (the "GC roots"). Compilers inject safe checks (polls) at method returns and loop back-edges.
2. **The Coordination Challenge:** 
   * When the GC decides to collect, it can't just pause threads individually; it must wait until **every single mutator** hits a safepoint and parks.
   * While parked, mutators must sleep (not spin-wait at 100% CPU).
   * When the GC finishes, it must wake all mutators up simultaneously.

This requires **two condition variable signals**:
* **Signal 1 (`notify_one`):** The *last* mutator to park tells the sleeping GC thread: *"Everyone is stopped; you can start collecting."*
* **Signal 2 (`notify_all`):** The GC thread tells all sleeping mutators: *"I'm done; everybody wake up and continue."*

---

### How It Works Step-by-Step

```
Mutator Threads (Application)                 GC / VM Thread
-----------------------------                 --------------
Running app code...                           Need to collect garbage!
                                              1. Set safepoint_active = true
                                              2. Lock mutex
                                              3. cv_all_stopped.wait(lock)
                                                 -> GC sleeps, waiting for mutators

4. Thread hits a safepoint poll
5. Sees safepoint_active == true
6. Locks mutex
7. Decrements active_mutator_count
8. If (active_mutator_count == 0):
      cv_all_stopped.notify_one()  ---------> 9. GC wakes up!
10. cv_resume.wait(lock,                      11. Runs GC (marks, moves, sweeps)
       []{ return !safepoint_active; })       12. Set safepoint_active = false
    -> Mutator goes to sleep                  13. cv_resume.notify_all()
                                                        |
14. ALL Mutators wake up simultaneously <---------------+
15. Increment active_mutator_count
16. Resume executing user code
```

---

### C++11 Code: A Mini Safepoint Barrier

Here is how a runtime's STW coordinator looks when implemented using C++11 primitives:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

class SafepointManager {
    std::mutex mtx;
    std::condition_variable cv_all_stopped; // GC waits on this
    std::condition_variable cv_resume;      // Mutators wait on this

    bool safepoint_active = false;          // The STW predicate
    int active_mutator_count = 0;           // Threads currently running code
    int total_mutators = 0;

public:
    void register_mutator() {
        std::lock_guard<std::mutex> lock(mtx);
        total_mutators++;
        active_mutator_count++;
    }

    // Called periodically by application threads (at loop ends, method returns)
    void safepoint_poll(int thread_id) {
        // Fast, lock-free check: if no STW is happening, keep running immediately
        if (!safepoint_active) return;

        // Slow path: STW requested! Park this thread.
        std::unique_lock<std::mutex> lock(mtx);
        
        active_mutator_count--;
        std::cout << "[Mutator " << thread_id << "] Reached safepoint and parked.\n";

        // If I am the LAST thread to arrive, wake up the waiting GC thread
        if (active_mutator_count == 0) {
            cv_all_stopped.notify_one();
        }

        // Put this thread to sleep until GC announces it is done
        cv_resume.wait(lock, [this] { return !safepoint_active; });

        // Resumed! Rejoin active execution
        active_mutator_count++;
        std::cout << "[Mutator " << thread_id << "] Resuming execution.\n";
    }

    // Called by the GC thread to execute a Stop-The-World pause
    void stop_the_world_and_collect() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            std::cout << "\n[GC] Triggering Stop-The-World pause...\n";
            safepoint_active = true;

            // Wait until every mutator has checked in and parked
            cv_all_stopped.wait(lock, [this] { return active_mutator_count == 0; });
        }

        // --- WORLD IS STOPPED ---
        // All threads are frozen at safe spots.
        // The GC can now safely trace roots and move memory without race conditions.
        std::cout << "[GC] All threads stopped. Performing garbage collection...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate GC work
        std::cout << "[GC] Collection complete!\n";
        // ------------------------

        {
            std::lock_guard<std::mutex> lock(mtx);
            safepoint_active = false;
        }

        // Broadcast to ALL parked threads so they resume concurrently
        cv_resume.notify_all();
    }
};
```

---

### Why the Condition Variable Mechanics Matter Here

#### 1. Why `cv_resume.notify_all()` instead of `notify_one()`?
When the GC finishes its work, you have dozens of application threads sleeping on `cv_resume`. If you called `notify_one()`, only a single thread would wake up. `notify_all()` acts as the **"starting gun"** that releases the entire barrier at once.

#### 2. Why the Predicate `!safepoint_active` is mandatory (Spurious Wakeups)
If an OS thread scheduler spuriously wakes up a mutator thread while the GC is still halfway through moving objects around in memory, that thread would read corrupted memory. 

Because `wait()` checks `!safepoint_active`, the thread sees the GC is still working and goes right back to sleep.

#### 3. Handling Native / JNI Calls (Lost Wakeup Prevention)
What happens if a thread is blocked inside a long OS system call (like `read()` or `sleep()`) when GC starts?
* That thread is **already at a safepoint** because it is not actively modifying the managed heap. The GC considers it "parked" while it is in native code.
* But when that thread returns from native code back into managed code, it must hit a safepoint check. Because of the predicate flag (`safepoint_active`), if the GC is still running, the returning thread immediately blocks on the condition variable before touching any heap pointers.

