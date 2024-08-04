#include <memory>

#include "InvalidStateException.h"

#include "JavaString.h"
//#include "JavaNullReference.h"
#include "JavaCodeAttribute.h"
#include "AttributeConstantValue.h"
#include "ConstantPool.h"

#include "TypeParser.h"
#include "GlobalCatalog.h"

#include "FieldInfo.h"

FieldInfo::FieldInfo( const ConstantPool &pPool, uint16_t flags, boost::intrusive_ptr<JavaString> name, boost::intrusive_ptr<JavaString> descriptor, CodeAttributeList attributes, intptr_t offset )
  : m_Name( name )
  , m_Descriptor( descriptor )
  , m_Attributes( attributes )
  , m_Prepared( false )
  , m_Offset( offset )
{
  m_Flags.m_FlagsAsInt = flags;

  if ( IsStatic() )
  {
    for ( const std::shared_ptr<JavaCodeAttribute> pAttribute : m_Attributes )
    {
      if ( pAttribute->GetType() == e_JavaAttributeTypeConstantValue )
      {
        m_pStaticValue = pPool.GetConstant( pAttribute->ToConstantValue().GetValueIndex() )->AsJavaVariable();
        if ( nullptr == m_pStaticValue )
        {
          throw InvalidStateException( __FUNCTION__ " - Expected some value here." );
        }
      }
    }

    if ( nullptr == m_pStaticValue )
    {
      m_pStaticValue = TypeParser::GetDefaultValue( *m_Descriptor );
    }
  }
}

FieldInfo::FieldInfo( const FieldInfo &other )
  : m_Flags( other.m_Flags )
  , m_Name( other.m_Name )
  , m_Descriptor( other.m_Descriptor )
  , m_Attributes( other.m_Attributes )
  , m_pStaticValue( other.m_pStaticValue )
  , m_Prepared( other.m_Prepared )
  , m_Offset( other.m_Offset )
{}

FieldInfo::FieldInfo( FieldInfo &&other )
  : m_Flags( other.m_Flags )
  , m_Name( std::move( other.m_Name ) )
  , m_Descriptor( std::move( other.m_Descriptor ) )
  , m_Attributes( std::move( other.m_Attributes ) )
  , m_pStaticValue( std::move( other.m_pStaticValue ) )
  , m_Prepared( std::move( other.m_Prepared ) )
  , m_Offset( std::move( other.m_Offset ) )
{}

FieldInfo FieldInfo::operator=( FieldInfo other ) JVMX_NOEXCEPT
{
  swap( *this, other );
  return *this;
}

uint16_t FieldInfo::GetFlags() const JVMX_NOEXCEPT
{
  return m_Flags.m_FlagsAsInt;
}

const CodeAttributeList &FieldInfo::GetAttributes() const
{
  return m_Attributes;
}

size_t FieldInfo::GetByteSize() const
{
  switch ( TypeParser::ConvertTypeDescriptorToVariableType( m_Descriptor->At( 0 ) ) )
  {
    case e_JavaVariableTypes::Object:
      return sizeof( ObjectReference );
      break;
    case e_JavaVariableTypes::Char:
      return sizeof( JavaChar );
      break;
    case e_JavaVariableTypes::Byte:
      return sizeof( JavaByte );
      break;
    case e_JavaVariableTypes::Short:
      return sizeof( JavaShort );
      break;
    case e_JavaVariableTypes::Integer:
      return sizeof( JavaInteger );
      break;
    case e_JavaVariableTypes::Long:
      return sizeof( JavaLong );
      break;
    case e_JavaVariableTypes::Float:
      return sizeof( JavaFloat );
      break;
    case e_JavaVariableTypes::Double:
      return sizeof( JavaDouble );
      break;
    case e_JavaVariableTypes::String:
      return sizeof( JavaString );
      break;
    case e_JavaVariableTypes::NullReference:
      return sizeof( ObjectReference );
      break;
    case e_JavaVariableTypes::ReturnAddress:
      return sizeof( intptr_t );
      break;
    case e_JavaVariableTypes::ClassReference:
      return sizeof( JavaClassReference );
      break;
    case e_JavaVariableTypes::Bool:
      return sizeof( JavaBool );
      break;
    case e_JavaVariableTypes::Array:
      return sizeof( ObjectReference );
      break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Unknown type." );
      break;
  }

  return 0;
}

void FieldInfo::swap( FieldInfo &left, FieldInfo &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Flags, right.m_Flags );
  std::swap( left.m_Name, right.m_Name );
  std::swap( left.m_Descriptor, right.m_Descriptor );
  std::swap( left.m_Attributes, right.m_Attributes );
  std::swap( left.m_pStaticValue, right.m_pStaticValue );
  std::swap( left.m_Prepared, right.m_Prepared );
  std::swap( left.m_Offset, right.m_Offset );
}

boost::intrusive_ptr<JavaString> FieldInfo::GetName() const
{
  return m_Name;
}

boost::intrusive_ptr<JavaString> FieldInfo::GetType() const
{
  return m_Descriptor;
}

size_t FieldInfo::GetOffset() const
{
  return m_Offset;
}

bool FieldInfo::IsStatic() const JVMX_NOEXCEPT
{
  return 0 != ( m_Flags.m_FlagsAsInt & static_cast<uint16_t>( e_JavaFieldAccessFlags::Static ) );
}

ConstantPoolIndex FieldInfo::GetConstantValueIndex() const
{
  if ( !IsStatic() )
  {
    throw InvalidStateException( __FUNCTION__ " - This field is not static and therefor does not have a constant value." );
  }

  for ( const std::shared_ptr<JavaCodeAttribute> attribute : m_Attributes )
  {
    if ( e_JavaAttributeTypeConstantValue == attribute->GetType() )
    {
      return attribute->ToConstantValue().GetValueIndex();
    }
  }

  return 0;
}

bool FieldInfo::IsFinal() const JVMX_NOEXCEPT
{
  return 0 != ( m_Flags.m_FlagsAsInt & static_cast<uint16_t>( e_JavaFieldAccessFlags::Final ) );
}

void FieldInfo::SetStaticValue( boost::intrusive_ptr<IJavaVariableType> pValue )
{
  if ( !IsStatic() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected this type to be static." );
  }

  m_pStaticValue = pValue;
}

boost::intrusive_ptr<IJavaVariableType> FieldInfo::GetStaticValue() const
{
  if ( !IsStatic() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected this type to be static." );
  }

  return m_pStaticValue;
}

bool FieldInfo::IsProtected() const JVMX_NOEXCEPT
{
  return 0 != ( m_Flags.m_FlagsAsInt & static_cast<uint16_t>( e_JavaFieldAccessFlags::Protected ) );
}

bool FieldInfo::IsPublic() const JVMX_NOEXCEPT
{
  return 0 != ( m_Flags.m_FlagsAsInt & static_cast<uint16_t>( e_JavaFieldAccessFlags::Public ) );
}

void FieldInfo::Prepare()
{
  if ( m_Prepared )
  {
    return;
  }

  //   if ( IsStatic() )
  //   {
  //     for ( auto attr : m_Attributes )
  //     {
  //       if ( attr->GetType() == e_JavaAttributeTypeConstantValue )
  //       {
  //         AttributeConstantValue value = attr->ToConstantValue();
  //         value->GetValueIndex()
  //       }
  //     }
  //   }

  m_Prepared = true;
}