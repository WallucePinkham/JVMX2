#include "JavaString.h"

#include "HelperTypes.h"

#include "CodeAttributeStackMapTable.h"
#include "ClassAttributeCode.h"
#include "JavaClass.h"

#include "MethodInfo.h"

MethodInfo::MethodInfo( uint16_t flags, boost::intrusive_ptr<JavaString> name, boost::intrusive_ptr<JavaString> descriptor, CodeAttributeList attributes )
  : m_Name( name )
  , m_Descriptor( descriptor )
  , m_Attributes( attributes )
  , m_pFrameInfo( nullptr )
  , m_pCodeInfo( nullptr )
  , m_pClass( nullptr )
{
  m_Flags.m_FlagsAsInt = flags;

  for ( auto attribute : m_Attributes )
  {
    if ( e_JavaAttributeTypeStackMapTable == attribute->GetType() )
    {
      m_pFrameInfo = dynamic_cast<CodeAttributeStackMapTable *>( attribute.get() );
    }

    if ( e_JavaAttributeTypeCode == attribute->GetType() )
    {
      m_pCodeInfo = dynamic_cast<ClassAttributeCode *>( attribute.get() );
    }
  }
}

MethodInfo::MethodInfo( const MethodInfo &other )
  : m_Flags( other.m_Flags )
  , m_Name( other.m_Name )
  , m_Descriptor( other.m_Descriptor )
  , m_Attributes( other.m_Attributes )
  , m_pFrameInfo( other.m_pFrameInfo )
  , m_pCodeInfo( other.m_pCodeInfo )
  , m_pClass( other.m_pClass )
{
  AssertValid();
}

MethodInfo::MethodInfo( MethodInfo &&other )
  : m_Flags( other.m_Flags )
  , m_Name( std::move( other.m_Name ) )
  , m_Descriptor( std::move( other.m_Descriptor ) )
  , m_Attributes( std::move( other.m_Attributes ) )
  , m_pFrameInfo( std::move( other.m_pFrameInfo ) )
  , m_pCodeInfo( std::move( other.m_pCodeInfo ) )
  , m_pClass( std::move( other.m_pClass ) )
{
  AssertValid();
}

MethodInfo MethodInfo::operator=( MethodInfo other ) JVMX_NOEXCEPT
{
  swap( *this, other );

  AssertValid(); // ?

  return *this;
}

uint16_t MethodInfo::GetFlags() const JVMX_NOEXCEPT
{
  AssertValid();

  return m_Flags.m_FlagsAsInt;
}

const CodeAttributeList &MethodInfo::GetAttributes() const
{
  AssertValid();

  return m_Attributes;
}

void MethodInfo::swap( MethodInfo &left, MethodInfo &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Flags, right.m_Flags );
  std::swap( left.m_Name, right.m_Name );
  std::swap( left.m_Descriptor, right.m_Descriptor );
  std::swap( left.m_Attributes, right.m_Attributes );
  std::swap( left.m_pFrameInfo, right.m_pFrameInfo );
  std::swap( left.m_pCodeInfo, right.m_pCodeInfo );
  std::swap( left.m_pClass, right.m_pClass );
}

boost::intrusive_ptr<JavaString> MethodInfo::GetName() const
{
  return m_Name;
}

boost::intrusive_ptr<JavaString> MethodInfo::GetType() const
{
  return m_Descriptor;
}

bool MethodInfo::IsStatic() const JVMX_NOEXCEPT
{
  AssertValid();
  return MatchFlag( e_JavaMethodAccessFlags::Static );
}

const CodeAttributeStackMapTable *MethodInfo::GetFrame() const JVMX_NOEXCEPT
{
  AssertValid();
  return m_pFrameInfo;
}

const ClassAttributeCode *MethodInfo::GetCodeInfo() const JVMX_NOEXCEPT
{
  AssertValid();
  return m_pCodeInfo;
}

bool MethodInfo::IsAbstract() const JVMX_NOEXCEPT
{
  AssertValid();
  return MatchFlag( e_JavaMethodAccessFlags::Abstract );
}

bool MethodInfo::IsSynchronised() const JVMX_NOEXCEPT
{
  AssertValid();
  return MatchFlag( e_JavaMethodAccessFlags::Synchronized );
}

bool MethodInfo::IsNative() const JVMX_NOEXCEPT
{
  AssertValid();
  return MatchFlag( e_JavaMethodAccessFlags::Native );
}

bool MethodInfo::IsProtected() const JVMX_NOEXCEPT
{
  AssertValid();
  return MatchFlag( e_JavaMethodAccessFlags::Protected );
}

bool MethodInfo::IsSignaturePolymorphic() const JVMX_NOEXCEPT
{
  // TODO: Fix this
  return false;
}

JavaClass *MethodInfo::GetClass() JVMX_NOEXCEPT
{
  AssertValid();
  return m_pClass;
}

const JavaClass *MethodInfo::GetClass() const JVMX_NOEXCEPT
{
  AssertValid();
  return m_pClass;
}

JavaString MethodInfo::GetPackageName() const JVMX_NOEXCEPT
{
  return HelperTypes::GetPackageNameFromClassName( *m_pClass->GetName() );
}

void MethodInfo::SetClass( JavaClass *pClass ) JVMX_NOEXCEPT
{
  m_pClass = pClass;
  AssertValid();
}

void MethodInfo::AssertValid() const
{
#ifdef _DEBUG
  JVMX_ASSERT( nullptr != m_pClass );
  JVMX_ASSERT( !m_pClass->GetName()->IsEmpty() );
#endif // DEBUG
}

bool MethodInfo::MatchFlag( e_JavaMethodAccessFlags flag ) const
{
  uint16_t flagToCheck = static_cast<uint16_t>( flag );
  return flagToCheck == ( m_Flags.m_FlagsAsInt & flagToCheck );
}

JavaString MethodInfo::GetFullName() const
{
  return ( *m_pClass->GetName() ).Append( u"::" ).Append( *GetName() ).Append( u"??" ).Append( *GetType() );
}
