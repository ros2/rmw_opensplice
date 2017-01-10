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

// copied from
// rosidl_typesupport_opensplice_cpp/resource/time__rosidl_typesupport_opensplice_cpp.hpp

// NOLINT(build/header_guard)
#ifndef BUILTIN_INTERFACES__MSG__TIME__ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_HPP_
#define BUILTIN_INTERFACES__MSG__TIME__ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_HPP_

#include <ccpp_dds_dcps.h>

#include "builtin_interfaces/msg/time__struct.hpp"

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"

#include "builtin_interfaces/msg/rosidl_typesupport_opensplice_cpp__visibility_control.h"

namespace builtin_interfaces
{
namespace msg
{
namespace typesupport_opensplice_cpp
{

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC_builtin_interfaces
extern void convert_ros_message_to_dds(
  const builtin_interfaces::msg::Time & ros_message,
  DDS::Time_t & dds_message);

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC_builtin_interfaces
extern void convert_dds_message_to_ros(
  const DDS::Time_t & dds_message,
  builtin_interfaces::msg::Time & ros_message);

}  // namespace typesupport_opensplice_cpp
}  // namespace msg
}  // namespace builtin_interfaces

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC_builtin_interfaces
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
  rosidl_typesupport_opensplice_cpp, builtin_interfaces, msg, Time)();

#ifdef __cplusplus
}
#endif

#endif  // BUILTIN_INTERFACES__MSG__TIME__ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_HPP_
