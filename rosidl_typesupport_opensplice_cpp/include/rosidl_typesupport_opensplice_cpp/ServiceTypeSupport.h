
#ifndef __rmw_opensplice_cpp__ServiceTypeSupport__h__
#define __rmw_opensplice_cpp__ServiceTypeSupport__h__

#include "rosidl_generator_cpp/ServiceTypeSupport.h"

namespace DDS {
class DomainParticipant;
}

namespace rmw
{

extern const char * _prismtech_opensplice_identifier;

}  // namespace rmw

namespace rmw_opensplice_cpp
{

typedef struct ServiceTypeSupportCallbacks {
  const char * _package_name;
  const char * _message_name;
  void* (*_create_client)(DDS::DomainParticipant * participant, const char * service_name);
  void* (*_create_service)(DDS::DomainParticipant * participant, const char * service_name);
} ServiceTypeSupportCallbacks;

template<typename T>
const rosidl_service_type_support_t * get_service_type_support_handle();

}  // namespace rmw_opensplice_cpp

#endif  // __rmw_opensplice_cpp__ServiceTypeSupport__h__
