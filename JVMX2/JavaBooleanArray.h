#pragma once
#include "JavaArray.h"

class JavaBooleanArray : public JavaArray
{
public:
    explicit JavaBooleanArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Boolean, size) {}

    JavaBool BoolAtIndex(size_t index) const { return BoolAt(index); }
    void SetBoolAtIndex(size_t index, const JavaBool& v) { SetAt(static_cast<uint32_t>(index), v); }
};