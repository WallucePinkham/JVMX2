#pragma once
#include "JavaArray.h"

class JavaDoubleArray : public JavaArray
{
public:
    explicit JavaDoubleArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Double, size) {}

    JavaDouble DoubleAtIndex(size_t index) const
    {
        const IJavaVariableType* p = GetValueAtIndex(index);
        return static_cast<const JavaDouble&>(*p);
    }

    void SetDoubleAtIndex(size_t index, const JavaDouble& v) { SetAt(static_cast<uint32_t>(index), v); }
};