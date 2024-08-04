
#ifndef _PARAMETERParameterAnnotationsEntry__H_
#define _PARAMETERParameterAnnotationsEntry__H_

#include "GlobalConstants.h"
#include "Stream.h"

#include "AnnotationsEntry.h"

class ParameterAnnotationsEntry
{
private:
  ParameterAnnotationsEntry();

public:
  ParameterAnnotationsEntry( const ParameterAnnotationsEntry &other );
  ParameterAnnotationsEntry( ParameterAnnotationsEntry &&other );

  virtual ~ParameterAnnotationsEntry() JVMX_NOEXCEPT;

  ParameterAnnotationsEntry &operator=(ParameterAnnotationsEntry other);

  bool operator==(const ParameterAnnotationsEntry &other) const JVMX_NOEXCEPT;

  uint16_t GetNumberOfAnnotations() const JVMX_NOEXCEPT;
  const std::shared_ptr<AnnotationsEntry> GetAnnotationAt( size_t index ) const;

  static std::shared_ptr<ParameterAnnotationsEntry> FromBinary( Stream &stream );

  virtual DataBuffer ToBinary() const;

protected:
  virtual void swap( ParameterAnnotationsEntry &left, ParameterAnnotationsEntry &right ) JVMX_NOEXCEPT;

private:
  AnnotationsList m_Annotations;
};

typedef std::vector<std::shared_ptr<ParameterAnnotationsEntry>> ParameterAnnotationsList;

#endif // _PARAMETERParameterAnnotationsEntry__H_
