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

#include "rosidl_typesupport_opensplice_cpp/Duration_TypeSupport.h"

namespace builtin_msgs
{
namespace type_support
{

void convert_ros_message_to_dds(
  const builtin_msgs::Duration & ros_message,
  DDS::Duration_t & dds_message)
{
  dds_message.sec = ros_message.sec;
  dds_message.nanosec = ros_message.nanosec;
}

void convert_dds_message_to_ros(
  const DDS::Duration_t & dds_message,
  builtin_msgs::Duration & ros_message)
{
  ros_message.sec = dds_message.sec;
  ros_message.nanosec = dds_message.nanosec;
}

}  // namespace type_support
}  // namespace builtin_msgs
