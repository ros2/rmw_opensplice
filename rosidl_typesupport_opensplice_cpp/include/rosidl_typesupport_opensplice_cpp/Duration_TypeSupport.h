
#ifndef __rosidl_typesupport_openslice_cpp__Duration_TypeSupport__h__
#define __rosidl_typesupport_openslice_cpp__Duration_TypeSupport__h__

#include <iostream>

#include "builtin_msgs/Duration_Struct.h"

namespace builtin_msgs
{
namespace type_support
{

void convert_ros_message_to_dds(const builtin_msgs::Duration& ros_message, DDS::Duration_t& dds_message)
{
  dds_message.sec = ros_message.sec;
  dds_message.nanosec = ros_message.nanosec;
}

void convert_dds_message_to_ros(const DDS::Duration_t& dds_message, builtin_msgs::Duration& ros_message)
{
  ros_message.sec = dds_message.sec;
  ros_message.nanosec = dds_message.nanosec;
}

}  // namespace type_support
}  // namespace builtin_msgs

#endif  // __rosidl_typesupport_openslice_cpp__Duration_TypeSupport__h__
