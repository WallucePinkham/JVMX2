# Runtime Access Control Specification & Test Design (JVMS §5.4.4)

## Overview
According to the Java Virtual Machine Specification (JVMS §5.4.4), runtime access control determines whether a running class or interface `D` has permission to access a referenced class, interface, field, or method `C`.

When access control fails at runtime (e.g., attempting to access a `private` or package-private member across unauthorized package or class boundaries), the JVM must throw an **`IllegalAccessError`**.

## Why Source-Level Tests Fall Short
Standard Java compilers (such as `javac`) enforce compile-time visibility and access checks. Attempting to write a Java source file that directly accesses a `private` member of another class or a package-private member across different packages results in a **compile-time error**, preventing the generation of `.class` bytecode files.

## Bytecode-Level Test Strategy (Failing Tests)
To properly test JVM runtime access control enforcement without compiler prevention, tests must be constructed at the bytecode level (or compiled with classes compiled independently and modified/assembled via bytecode tools like ASM/javap). 

### Test Scenario 1: Cross-Package Package-Private Access
- **Class `pkg1.Base`**: Defines a package-private field `int value = 42;`.
- **Class `pkg2.AccessViolation`**: In package `pkg2`, references `pkg1.Base` and attempts to read its package-private field.
- **Expected Outcome**: When executed by the JVM, resolution of the field must verify package alignment and throw `java.lang.IllegalAccessError` (currently unimplemented / bypassed in certain execution paths).

### Test Scenario 2: Private Method/Field Access Across Class Boundaries
- **Class `Target`**: Defines `private static void restrictedMethod() {}`.
- **Class `Attacker`**: Attempts `invokestatic Target.restrictedMethod`.
- **Expected Outcome**: Method resolution must check access flags (`ACC_PRIVATE`) and throw `java.lang.IllegalAccessError`.
