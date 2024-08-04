
#pragma once

#ifndef __CONSTANTPOOL_H__
#define __CONSTANTPOOL_H__

#include <memory>
#include <vector>

#include "GlobalConstants.h"

#include "ConstantPoolEntry.h"

class ConstantPoolEntry;

class ConstantPool
{
public:
  ConstantPool();

  ConstantPool( const ConstantPool &other );
  ConstantPool( ConstantPool &&other );

  virtual ~ConstantPool();

  virtual size_t GetCount() const;

  virtual void AddConstant( std::shared_ptr<ConstantPoolEntry> pConstant );

  virtual std::shared_ptr<ConstantPoolEntry> GetConstant( size_t index ) const;

  virtual size_t GetLastValidIndex() const;

  // Returns 0 (invalid index) if the string constant could not be found.
  virtual ConstantPoolIndex GetStringConstantIndex( const JavaString &stringConstant ) const;

  virtual void Prepare();

public:
  static size_t GetFirstValidIndex();

// #ifdef _DEBUG
// public:
//   void DebugDump();
// #endif // _DEBUG

private:
  ConstantPool& operator=(const ConstantPool &other);
  ConstantPool& operator=(ConstantPool &&other);

protected:
  typedef std::vector<std::shared_ptr<ConstantPoolEntry>> ConstantPoolEntryList;

private:
  ConstantPoolEntryList m_Entries;
};

#endif // __ConstantPool_H__


