#include "StringPool.h"


void StringPool::Add(const JavaString& key, boost::intrusive_ptr<ObjectReference> value)
{
  m_Pool[key.ToUtf16String()] = value;
}

boost::intrusive_ptr<ObjectReference> StringPool::Get(const JavaString& key)
{
  auto pos = m_Pool.find(key.ToUtf16String());
  if (pos == m_Pool.cend())
  {
    return nullptr;
  }

  return pos->second;
}

std::vector<boost::intrusive_ptr<ObjectReference>> StringPool::GetAll()
{
  std::vector<boost::intrusive_ptr<ObjectReference>> result;

  for (auto string : m_Pool)
  {
    result.push_back(string.second);
  }

  return result;
}