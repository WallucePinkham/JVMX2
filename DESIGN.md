# JVMX2 Design Architecture

## Core Principles
- **Immutability & Thread Safety**: Classes are designed to be immutable wherever possible to facilitate safe multi-threading and state management across virtual machine threads.
- **C++11 Strictness**: Strictly adheres to C++11 standards (no C++14/17/20 features, no `std::make_unique`, no precompiled headers).
- **Modularity**: Separation of concerns across execution engine modules, class loaders, memory managers, and garbage collectors.

## Subsystems

### 1. Execution Engine (`BasicExecutionEngine`)
The execution engine drives the JVM bytecode interpretation loop (`Run()`). To maintain clean separation, it is modularized across specialized translation compilation units:
- `BasicExecutionEngine_Math.cpp`: Arithmetic and bitwise operations (`lneg`, `fneg`, `lushr`, etc.).
- `BasicExecutionEngine_LoadsStores.cpp`: Variable loading, storing, type conversions (`l2f`, `f2l`, `d2l`), and stack swapping (`swap`).
- `BasicExecutionEngine_ControlFlow.cpp`: Branching and jump routines.
- `BasicExecutionEngine_Invoke.cpp`: Method dispatch, resolution, and native method invocation.
- `BasicExecutionEngine_Objects.cpp`: Object creation and field manipulation.

### 2. Memory & Virtual Machine State (`IVirtualMachineState`, `BasicVirtualMachineState`, `BasicStackManager`)
- Manages operand stacks, local variable tables, call stacks, and thread states.
- Uses RAII (`std::unique_ptr`, `std::shared_ptr`) for strict memory lifetime safety without manual `delete`.

### 3. Class Loading & Resolution (`DefaultClassLoader`, `ConstantPool`, `StringPool`)
- Parses Java class files (Java SE 7 specification).
- Resolves constant pool entries, methods, fields, and classes.
- Caches interned strings and objects in the `StringPool`.

### 4. Garbage Collection (`CheneyGarbageCollector`, `DefaultGarbageCollector`)
- Manages heap allocations and object reference tracing / Cheney collection cycles.

### 5. Access Control & Verification (JVMS §5.4.4)
- Enforces member visibility and access checks during runtime linking and method/field resolution, throwing `IllegalAccessError` on violations.
