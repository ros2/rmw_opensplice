
#ifndef __ros_middleware_opensplice_cpp__ServiceTypeSupport__h__
#define __ros_middleware_opensplice_cpp__ServiceTypeSupport__h__

#include "rosidl_generator_cpp/ServiceTypeSupport.h"

namespace DDS {
class DomainParticipant;
}

namespace ros_middleware_interface
{

extern const char * _prismtech_opensplice_identifier;

}  // namespace ros_middleware_interface

namespace ros_middleware_opensplice_cpp
{

typedef struct ServiceTypeSupportCallbacks {
  const char * _package_name;
  const char * _message_name;
  void* (*_create_client)(DDS::DomainParticipant * participant, const char * service_name);
  void* (*_create_service)(DDS::DomainParticipant * participant, const char * service_name);
} ServiceTypeSupportCallbacks;

template<typename T>
const rosidl_generator_cpp::ServiceTypeSupportHandle& get_service_type_support_handle();

}  // namespace ros_middleware_opensplice_cpp

#endif  // __ros_middleware_opensplice_cpp__ServiceTypeSupport__h__
