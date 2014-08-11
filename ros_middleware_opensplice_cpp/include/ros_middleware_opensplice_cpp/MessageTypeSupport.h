
#ifndef __ros_middleware_opensplice_cpp__MessageTypeSupport__h__
#define __ros_middleware_opensplice_cpp__MessageTypeSupport__h__

#include "rosidl_generator_cpp/MessageTypeSupport.h"

namespace DDS {
class DomainParticipant;
class DataWriter;
}

namespace ros_middleware_interface
{

extern const char * _prismtech_opensplice_identifier;

}  // namespace ros_middleware_interface

namespace ros_middleware_opensplice_cpp
{

typedef struct MessageTypeSupportCallbacks {
  const char * _package_name;
  const char * _message_name;
  void (*_register_type)(DDS::DomainParticipant * participant, const char * type_name);
  void (*_publish)(DDS::DataWriter * topic_writer, const void * ros_message);
} MessageTypeSupportCallbacks;

template<typename T>
const rosidl_generator_cpp::MessageTypeSupportHandle& get_type_support_handle();

}  // namespace ros_middleware_opensplice_cpp

#endif  // __ros_middleware_opensplice_cpp__MessageTypeSupport__h__
