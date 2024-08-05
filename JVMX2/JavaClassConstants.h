
#ifndef __JAVACLASSCONSTANTS_H__
#define __JAVACLASSCONSTANTS_H__

#include <vector>
#include <memory>

class ConstantPoolClassReference;  // Forward Declaration
class ConstantPoolFieldReference;  // Forward Declaration
class ConstantPoolMethodReference; // Forward Declaration
class JavaCodeAttribute; // Forward Declaration
class JavaCodeAttribute;  // Forward Declaration

enum class e_JavaClassAccessFlags : uint16_t
{
  None
  , Public = 0x01       // Accessible outside the package
  , Final = 0x10        // No subclasses allowed
  , Super = 0x20        // Treat superclass methods specially when invoked by the invokespecial instruction
  , Interface = 0x200   // An interface, not a class
  , Abstract = 0x400    // Must not be instantiated
  , Synthetic = 0x1000  // Generated by the compiler. Does not appear in the source.
  , Annotation = 0x2000
  , Enum = 0x4000
};

typedef std::vector<ConstantPoolClassReference> ClassReferenceList;
typedef std::vector<ConstantPoolFieldReference> FieldReferenceList;
typedef std::vector<ConstantPoolMethodReference> MethodReferenceList;
typedef std::vector<std::shared_ptr<JavaCodeAttribute> > CodeAttributeList;

#endif // __JAVACLASSCONSTANTS_H__
