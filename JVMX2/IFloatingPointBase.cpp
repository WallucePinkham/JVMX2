
#include <limits>
#include <cfloat>
#include <memory>
#include <sstream>

#include "JavaTypes.h"

#include "InternalErrorException.h"

#include "IFloatingPointBase.h"

static const int c_SignificantDigits = 12;

bool IFloatingPointBase::operator<(const IJavaVariableType &other) const
{
  return IJavaVariableType::operator<(other);
}

bool IFloatingPointBase::operator==(const IJavaVariableType &other) const
{
  return IJavaVariableType::operator==(other);
}

bool IFloatingPointBase::operator!=(const IJavaVariableType &other) const
{
  return IJavaVariableType::operator!=(other);
}

JavaString IFloatingPointBase::ConvertDoubleToString( double value )
{
  if ( std::isnan( value ) )
  {
    return JavaString::FromCString( u"NaN" );
  }

  if ( 0.0 == value )
  {
    return JavaString::FromCString( u"0.0" );
  }

  /// I was doing this on the stack, but I decided it would be safer to do it on the heap.
  /// There is always the possibility that this is called in a recursive function.
  /// Also, this is now exception safe.

  //char pDest[ 6+12+1 ];
  //memset( pDest, 0, 20 );

  //errno_t err = _gcvt_s( pDest, 20, value, c_SignificantDigits );

  //snprintf( pDest, 6 + 12, "%6.12f", value );

  std::string result;
  int64_t iPart = (int64_t)(value);
  double fPart = value - (double)(iPart);

  std::stringstream sstr;
  if ( value < 0.0 )
  {
    sstr << "-";
    value *= -1;
  }
  
  sstr << iPart << '.';

  for ( int64_t i = 0; i < 12; ++ i )
  {
    value *= 10;
    iPart = (int64_t)(value);
    sstr << iPart;

    value -= iPart;

    if ( 0 == value )
    {
      break;
    }
  }


//   if ( 0 != err )
//   {
//     throw InternalErrorException( __FUNCTION__ " - Could not convert double value to a string.", err );
//   }

  return JavaString::FromCString( sstr.str().c_str() );
}
