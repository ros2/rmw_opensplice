// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include "rosidl_typesupport_opensplice_c/duration__type_support.h"

#include <ccpp_dds_dcps.h>

#include <builtin_interfaces/msg/duration__struct.h>
#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>
#include <rosidl_typesupport_opensplice_c/identifier.h>

static const char *
convert_ros_to_dds(const void * untyped_ros_message, void * untyped_dds_message)
{
  if (!untyped_ros_message) {
    return "ros message is null";
  }
  if (!untyped_dds_message) {
    return "dds message is null";
  }
  const builtin_interfaces__msg__Duration * ros_message =
    static_cast<const builtin_interfaces__msg__Duration *>(untyped_ros_message);
  DDS::Duration_t * dds_message = static_cast<DDS::Duration_t *>(untyped_dds_message);
  dds_message->sec = ros_message->sec;
  dds_message->nanosec = ros_message->nanosec;
  return 0;
}

static const char *
convert_dds_to_ros(const void * untyped_dds_message, void * untyped_ros_message)
{
  if (!untyped_dds_message) {
    return "dds message is null";
  }
  if (!untyped_ros_message) {
    return "ros message is null";
  }
  const DDS::Duration_t * dds_message = static_cast<const DDS::Duration_t *>(untyped_dds_message);
  builtin_interfaces__msg__Duration * ros_message =
    static_cast<builtin_interfaces__msg__Duration *>(untyped_ros_message);
  ros_message->sec = dds_message->sec;
  ros_message->nanosec = dds_message->nanosec;
  return 0;
}

const rosidl_message_type_support_t *
ROSIDL_GET_TYPE_SUPPORT_FUNCTION(builtin_interfaces, msg, Duration)()
{
  static message_type_support_callbacks_t callbacks = {
    .package_name = "builtin_interfaces",
    .message_name = "Duration",
    .register_type = nullptr,
    .publish = nullptr,
    .take = nullptr,
    .convert_ros_to_dds = convert_ros_to_dds,
    .convert_dds_to_ros = convert_dds_to_ros,
  };
  static rosidl_message_type_support_t type_support = {
    .typesupport_identifier = rosidl_typesupport_opensplice_c__identifier,
    .data = &callbacks,
  };
  return &type_support;
}
