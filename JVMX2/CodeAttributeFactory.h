
#ifndef __CODEATTRIBUTEFACTORY_H__
#define __CODEATTRIBUTEFACTORY_H__

#include <memory>

class JavaCodeAttribute;
class ConstantPool;
class Stream;
class JavaString;

class CodeAttributeFactory
{
public:
  virtual std::shared_ptr<JavaCodeAttribute> CreateFromBinary( const JavaString &name, Stream &stream, const ConstantPool &constantPool, size_t byteCount );
};

#endif // __CODEATTRIBUTEFACTORY_H__

