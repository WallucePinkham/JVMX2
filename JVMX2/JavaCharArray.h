#pragma once
#include "JavaArray.h"

class JavaCharArray : public JavaArray
{
public:
    explicit JavaCharArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Char, size) {}

    JavaChar CharAtIndex(size_t index) const { return CharAt(index); }
    void SetCharAtIndex(size_t index, const JavaChar& v) { SetAt(static_cast<uint32_t>(index), v); }
};