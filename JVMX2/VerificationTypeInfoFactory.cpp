
#include "TypeMismatchException.h"
#include "VerificationTypeInfo.h"

#include "Stream.h"

#include "VerificationTypeInfoTop.h"
#include "VerificationTypeInfoInteger.h"
#include "VerificationTypeInfoFloat.h"
#include "VerificationTypeInfoDouble.h"
#include "VerificationTypeInfoLong.h"
#include "VerificationTypeInfoNull.h"
#include "VerificationTypeInfoConstant.h"
#include "VerificationTypeInfoUninitialised.h"
#include "VerificationTypeInfoUninitialisedThis.h"

#include "VerificationTypeInfoFactory.h"

VerificationTypeInfoFactory::~VerificationTypeInfoFactory() JVMX_NOEXCEPT
{
}

std::shared_ptr<VerificationTypeInfo> VerificationTypeInfoFactory::CreateVerificationTypeInfo( uint8_t tag, Stream &byteStream )
{
  e_VerificationTypeInfoTypes type = static_cast<e_VerificationTypeInfoTypes>(tag);

  switch ( type )
  {
    case e_VerificationTypeInfoTypes::Top:
      return std::make_shared<VerificationTypeInfoTop>();
      break;

    case e_VerificationTypeInfoTypes::Integer:
      return std::make_shared<VerificationTypeInfoInteger>();
      break;

    case e_VerificationTypeInfoTypes::Float:
      return std::make_shared<VerificationTypeInfoFloat>();
      break;

    case e_VerificationTypeInfoTypes::Double:
      return std::make_shared<VerificationTypeInfoDouble>();
      break;

    case e_VerificationTypeInfoTypes::Long:
      return std::make_shared<VerificationTypeInfoLong>();
      break;

    case e_VerificationTypeInfoTypes::Null:
      return std::make_shared<VerificationTypeInfoNull>();
      break;

    case e_VerificationTypeInfoTypes::UninitialisedThis:
      return std::make_shared<VerificationTypeInfoUninitialsedThis>();
      break;

    case e_VerificationTypeInfoTypes::Object:
      return std::make_shared<VerificationTypeInfoConstant>( byteStream.ReadUint16() );
      break;

    case e_VerificationTypeInfoTypes::Uninitialised:
      return std::make_shared<VerificationTypeInfoUninitialised>( byteStream.ReadUint16() );
      break;

    default:
      throw TypeMismatchException( __FUNCTION__ " - Unexpected value found for type verification info." );
      break;
  }
}
