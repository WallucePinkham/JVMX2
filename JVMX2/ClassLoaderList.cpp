#include "ClassLoaderList.h"

#include "InvalidArgumentException.h"
#include "NullPointerException.h"
#include "BasicClassLibrary.h"

void ClassLoaderList::AddClassLoader(boost::intrusive_ptr<ObjectReference> pClassLoader)
{
    if (pClassLoader == nullptr)
    {
        throw NullPointerException(__FUNCTION__ " - ClassLoader pointer is NULL.");
    }
    m_classLoaders[pClassLoader] = std::make_shared< BasicClassLibrary>();
}

std::shared_ptr<JavaClass> ClassLoaderList::FindLoadedClass(boost::intrusive_ptr<ObjectReference> pClassLoader, const JavaString& className)
{
  if (pClassLoader == nullptr)
  {
    throw NullPointerException(__FUNCTION__ " - ClassLoader pointer is NULL.");
  }
  
  auto it = m_classLoaders.find(pClassLoader);
  
  if (it != m_classLoaders.end())
  {
    return it->second->FindClass(className);
  }

  AddClassLoader(pClassLoader);
  return nullptr;
}

std::shared_ptr<JavaClass> ClassLoaderList::AddLoadedClass(boost::intrusive_ptr<ObjectReference> pClassLoader, std::shared_ptr<JavaClass> pClass)
{
  if (pClassLoader == nullptr)
  {
    throw NullPointerException(__FUNCTION__ " - ClassLoader pointer is NULL.");
  }

  auto it = m_classLoaders.find(pClassLoader);

  if (it == m_classLoaders.end())
  {
    AddClassLoader(pClassLoader);
  }

  m_classLoaders[pClassLoader]->AddClass(pClass);
}

std::shared_ptr<JavaClass> ClassLoaderList::FindInAnyClassLoader(const JavaString& className) const
{
    for (const auto& loader : m_classLoaders)
    {
      auto pClass = loader.second->FindClass(className);
      if (pClass)
      {
        return pClass;
      }
    }
    return nullptr;
}


