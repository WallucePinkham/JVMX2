#include "JavaFloatArray.h"

inline JavaFloatArray::JavaFloatArray(size_t size)
    : JavaArray(e_JavaArrayTypes::Float, size) {
}

inline JavaFloat JavaFloatArray::FloatAtIndex(size_t index) const
{
  const IJavaVariableType* p = GetValueAtIndex(index);
  return static_cast<const JavaFloat&>(*p);
}

inline void JavaFloatArray::SetFloatAtIndex(size_t index, const JavaFloat& v) { SetAt(static_cast<uint32_t>(index), v); }
