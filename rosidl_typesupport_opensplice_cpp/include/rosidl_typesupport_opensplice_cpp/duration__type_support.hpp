// Copyright 2014 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __rosidl_typesupport_openslice_cpp__duration__type_support__h__
#define __rosidl_typesupport_openslice_cpp__duration__type_support__h__

#include <ccpp_dds_dcps.h>

#include <rosidl_typesupport_opensplice_cpp/visibility_control.h>

#include "builtin_msgs/msg/duration__struct.hpp"

namespace builtin_msgs
{
namespace msg
{
namespace type_support
{

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC
extern void convert_ros_message_to_dds(
  const builtin_msgs::msg::Duration & ros_message,
  DDS::Duration_t & dds_message);

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC
extern void convert_dds_message_to_ros(
  const DDS::Duration_t & dds_message,
  builtin_msgs::msg::Duration & ros_message);

}  // namespace type_support
}  // namespace msg
}  // namespace builtin_msgs

#endif  // __rosidl_typesupport_openslice_cpp__duration__type_support__h__
