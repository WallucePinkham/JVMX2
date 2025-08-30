#pragma once
#include "JavaArray.h"

class JavaIntegerArray : public JavaArray
{
public:
    explicit JavaIntegerArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Integer, size) {}

    JavaInteger IntAtIndex(size_t index) const
    {
        // default implementation delegates to base At() and casts
        const IJavaVariableType* p = GetValueAtIndex(index);
        // caller's codebase defines JavaInteger wrapper; assume convertable
        return static_cast<const JavaInteger&>(*p);
    }

    void SetIntAtIndex(size_t index, const JavaInteger& v) { SetAt(static_cast<uint32_t>(index), v); }
};