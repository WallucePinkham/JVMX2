#ifndef _METHODINFO__H_
#define _METHODINFO__H_

#include <map>

#include "GlobalConstants.h"
#include "ConstantPoolMethodReference.h"
#include "JavaClassConstants.h"
#include "JavaString.h"

class CodeAttributeStackMapTable;
class ClassAttributeCode;

enum class e_JavaMethodAccessFlags : uint16_t
{
  Public = 0x01       // Accessible outside the package
  , Private = 0x2       // Only accessible in the defining class
  , Protected = 0x4     // May be accessed within subclasses
  , Static = 0x8        // static
  , Final = 0x10        // Must not be overridden
  , Synchronized = 0x20 // Invocation is wrapped by monitor use
  , Bridge = 0x40       // Bridge method generated by the compiler
  , VarArgs = 0x80      // Declared with variable number of arguments.
  , Native = 0x100      // Implemented in language other than Java
  , Abstract = 0x400    // No implementation is provided
  , Strict = 0x800      // declared strictfp. Floating point mode is strict
  , Synthetic = 0x1000  // Generated by the compiler. Does not appear in the source.
};

class MethodInfo
{
public:
  MethodInfo( uint16_t flags, boost::intrusive_ptr<JavaString> name, boost::intrusive_ptr<JavaString> descriptor, CodeAttributeList attributes );

  MethodInfo( const MethodInfo &other );
  MethodInfo( MethodInfo &&other );

  MethodInfo operator=( MethodInfo other ) JVMX_NOEXCEPT;

  virtual uint16_t GetFlags() const JVMX_NOEXCEPT;
  virtual boost::intrusive_ptr<JavaString> GetName() const;
  virtual boost::intrusive_ptr<JavaString> GetType() const;
  virtual const CodeAttributeList &GetAttributes() const;

  static void swap( MethodInfo &left, MethodInfo &right ) JVMX_NOEXCEPT;
  virtual bool IsStatic() const JVMX_NOEXCEPT;
  virtual bool IsAbstract() const JVMX_NOEXCEPT;
  virtual bool IsSynchronised() const JVMX_NOEXCEPT;

  virtual bool IsNative() const JVMX_NOEXCEPT;
  virtual bool IsProtected() const JVMX_NOEXCEPT;
  virtual bool IsSignaturePolymorphic() const JVMX_NOEXCEPT;

  virtual const CodeAttributeStackMapTable *GetFrame() const JVMX_NOEXCEPT;
  virtual const ClassAttributeCode *GetCodeInfo() const JVMX_NOEXCEPT;

  virtual void SetClass( JavaClass *pClass ) JVMX_NOEXCEPT;
  virtual JavaClass *GetClass() JVMX_NOEXCEPT;
  virtual const JavaClass *GetClass() const JVMX_NOEXCEPT;

  JavaString GetPackageName() const JVMX_NOEXCEPT;

  JavaString GetFullName() const;

private:
  void AssertValid() const;

  bool MatchFlag( e_JavaMethodAccessFlags type ) const;

private:

private:
  struct FlagBits
  {
    uint16_t flagPublic : 1;
    uint16_t flagPrivate : 1;
    uint16_t flagProtected : 1;
    uint16_t flagStatic : 1;
    uint16_t flagFinal : 1;
    uint16_t flagSynchronized : 1;
    uint16_t flagBridge : 1;
    uint16_t flagVarArgs : 1;
    uint16_t flagNative : 1;
    uint16_t flagReserved1 : 1;
    uint16_t flagAbstract : 1;
    uint16_t flagStrict : 1;
    uint16_t flagSynthetic : 1;
  };

  union
  {
    uint16_t m_FlagsAsInt;
    FlagBits m_FlagsAsBits;
  } m_Flags;

  //uint16_t ;
  boost::intrusive_ptr<JavaString> m_Name;
  boost::intrusive_ptr<JavaString> m_Descriptor;
  CodeAttributeList m_Attributes;
  const CodeAttributeStackMapTable* m_pFrameInfo;
  const ClassAttributeCode* m_pCodeInfo;

  JavaClass *m_pClass;
};

typedef std::multimap< JavaString, std::shared_ptr<MethodInfo> > MethodInfoList;

#endif // _METHODINFO__H_