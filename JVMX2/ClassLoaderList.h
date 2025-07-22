#pragma once
#ifndef _CLASSLOADERLIST__H_
#define _CLASSLOADERLIST__H_

#include <boost/intrusive_ptr.hpp>
#include <map>
#include <memory>

#include <wallaroo/part.h>

#include "IClassLibrary.h"
#include "JavaClass.h"
#include "JavaObject.h"
#include "JavaString.h"
#include "ObjectReference.h"


class ClassLoaderList : public wallaroo::Part
{
public:
  ClassLoaderList() = default;

  void AddClassLoader(boost::intrusive_ptr<ObjectReference> pClassLoader);
  std::shared_ptr<JavaClass> FindLoadedClass(boost::intrusive_ptr<ObjectReference> pClassLoader, const JavaString& className);
  std::shared_ptr<JavaClass> AddLoadedClass(boost::intrusive_ptr<ObjectReference> pClassLoader, std::shared_ptr<JavaClass> pClass);
  std::shared_ptr<JavaClass> FindInAnyClassLoader(const JavaString& className) const;


private:
  ClassLoaderList( const ClassLoaderList &other ) JVMX_FN_DELETE;
  ClassLoaderList& operator=(const ClassLoaderList& other) JVMX_FN_DELETE;

private:
  std::map<boost::intrusive_ptr<ObjectReference>, std::shared_ptr<IClassLibrary> > m_classLoaders;
};


#endif // _CLASSLOADERLIST__H_