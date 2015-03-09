#ifndef __rmw_opensplice_cpp__MessageTypeSupport__h__
#define __rmw_opensplice_cpp__MessageTypeSupport__h__

#include "rosidl_generator_cpp/MessageTypeSupport.h"

namespace DDS {
class DomainParticipant;
class DataReader;
class DataWriter;
}

namespace rmw
{

extern const char * _prismtech_opensplice_identifier;

}  // namespace rmw

namespace rmw_opensplice_cpp
{

template<typename T>
const rosidl_message_type_support_t * get_type_support_handle();

}  // namespace rmw_opensplice_cpp

#endif  // __rmw_opensplice_cpp__MessageTypeSupport__h__
