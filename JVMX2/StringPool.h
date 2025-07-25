#ifndef _STRINGPOOL__H_
#define _STRINGPOOL__H_

#include <map>

#include "JavaString.h"
#include "ObjectReference.h"
#include "CheneyGarbageCollector.h"

class StringPool : public wallaroo::Part
{
public:
  void Add(const JavaString& key, boost::intrusive_ptr<ObjectReference> value);
  boost::intrusive_ptr<ObjectReference> Get(const JavaString& key);

  // Mostly intended for use by Garbage Collector
  std::vector<boost::intrusive_ptr<ObjectReference>> GetAll();

private:
  std::map<std::u16string, boost::intrusive_ptr<ObjectReference>> m_Pool;
};

#endif // !_STRINGPOOL__H_

