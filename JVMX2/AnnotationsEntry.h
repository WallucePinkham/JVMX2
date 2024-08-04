
#ifndef _RUNTIMEVISIBLEANNOTATIONSENTRY__H_
#define _RUNTIMEVISIBLEANNOTATIONSENTRY__H_

#include <vector>
#include <memory>

#include "GlobalConstants.h"
#include "Stream.h"

#include "AnnotationElementValue.h"

class AnnotationsEntry
{
private:
  AnnotationsEntry( ConstantPoolIndex typeIndex );

public:
  AnnotationsEntry( const AnnotationsEntry &other );
  AnnotationsEntry( AnnotationsEntry &&other );

  virtual ~AnnotationsEntry() JVMX_NOEXCEPT;

  AnnotationsEntry &operator=(AnnotationsEntry other);

  bool operator==(const AnnotationsEntry &other) const JVMX_NOEXCEPT;

  uint16_t GetTypeIndex() const JVMX_NOEXCEPT;
  uint16_t GetNumberOfElementValuePairs() const JVMX_NOEXCEPT;
  const AnnotationsElementValuePair &GetElementValuePairAt( size_t index ) const;

  static std::shared_ptr<AnnotationsEntry> FromBinary( Stream &stream );

  virtual DataBuffer ToBinary() const;

  static AnnotationsEntry BlankEntry();

protected:
  virtual void swap( AnnotationsEntry &left, AnnotationsEntry &right ) JVMX_NOEXCEPT;

private:
  ConstantPoolIndex m_TypeIndex;
  AnnotationsElementValuePairList m_ElementValuePairs;
};

typedef std::vector<std::shared_ptr<AnnotationsEntry>> AnnotationsList;

#endif // _RUNTIMEVISIBLEANNOTATIONSENTRY__H_
