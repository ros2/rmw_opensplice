
#ifndef __rmw_opensplice_cpp__ServiceTypeSupport__h__
#define __rmw_opensplice_cpp__ServiceTypeSupport__h__

#include "rosidl_generator_cpp/ServiceTypeSupport.h"

namespace DDS {
class DomainParticipant;
class DataReader;
class DataWriter;
}

namespace rmw_opensplice_cpp
{

template<typename T>
const rosidl_service_type_support_t * get_service_type_support_handle();

}  // namespace rmw_opensplice_cpp

#endif  // __rmw_opensplice_cpp__ServiceTypeSupport__h__
