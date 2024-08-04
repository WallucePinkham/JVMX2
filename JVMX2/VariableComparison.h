
#ifndef _VARIABLECOMPARISON__H_
#define _VARIABLECOMPARISON__H_

#include <memory>

class IJavaVariableType;
class JavaString;


class CompareSharedVariablePointers
{
public:
  bool operator()( boost::intrusive_ptr<IJavaVariableType> left, boost::intrusive_ptr<IJavaVariableType> right ) const
  {
    return *left < *right;
  }
};

// class CompareSharedStringPointers
// {
// public:
//   bool operator()( const boost::intrusive_ptr<JavaString> left, const boost::intrusive_ptr<JavaString> right ) const
//   {
//     return *(left.get()) < *(right.get());
//   }
// };


#endif // _VARIABLECOMPARISON__H_
