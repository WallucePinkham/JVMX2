#pragma once
#include "JavaArray.h"

class JavaFloatArray : public JavaArray
{
public:
    explicit JavaFloatArray(size_t size);

    JavaFloat FloatAtIndex(size_t index) const;

    void SetFloatAtIndex(size_t index, const JavaFloat& v);
};