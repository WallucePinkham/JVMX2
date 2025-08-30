#pragma once
#include "JavaArray.h"

class JavaByteArray : public JavaArray
{
public:
    explicit JavaByteArray(size_t size)
        : JavaArray(e_JavaArrayTypes::Byte, size) {}

    // Convenience typed API (calls base implementations)
    JavaByte ByteAtIndex(size_t index) const { return ByteAt(index); }
    void SetByteAtIndex(size_t index, const JavaByte& v) { SetAt(static_cast<uint32_t>(index), v); }
};