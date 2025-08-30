#pragma once
#include "JavaArray.h"

class JavaLongArray : public JavaArray
{
public:
    explicit JavaLongArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Long, size) {}

    JavaLong LongAtIndex(size_t index) const
    {
        const IJavaVariableType* p = GetValueAtIndex(index);
        return static_cast<const JavaLong&>(*p);
    }

    void SetLongAtIndex(size_t index, JavaLong v) { SetAt(static_cast<uint32_t>(index), v); }
};