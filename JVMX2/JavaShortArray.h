#pragma once
#include "JavaArray.h"

class JavaShortArray : public JavaArray
{
public:
    explicit JavaShortArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Short, size) {}

    JavaShort ShortAtIndex(size_t index) const
    {
        const IJavaVariableType* p = GetValueAtIndex(index);
        return static_cast<const JavaShort&>(*p);
    }

    void SetShortAtIndex(size_t index, const JavaShort& v) { SetAt(static_cast<uint32_t>(index), v); }
};