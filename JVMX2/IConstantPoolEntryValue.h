
#pragma once

#ifndef __ICONSTANTPOOLENTRYVALUE_H__
#define __ICONSTANTPOOLENTRYVALUE_H__

class ConstantPool; // Forward declaration

class IConstantPoolEntryValue
{
public:
  virtual ~IConstantPoolEntryValue() JVMX_NOEXCEPT{};

  virtual void Prepare( const ConstantPool * ) {};

private:
  const IConstantPoolEntryValue &operator=(const IConstantPoolEntryValue &other);
};


#endif // __ICONSTANTPOOLENTRYVALUE_H__
