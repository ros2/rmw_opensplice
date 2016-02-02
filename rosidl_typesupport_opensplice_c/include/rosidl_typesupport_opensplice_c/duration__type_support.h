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

#ifndef ROSIDL_TYPESUPPORT_OPENSPLICE_C__DURATION__TYPE_SUPPORT_H_
#define ROSIDL_TYPESUPPORT_OPENSPLICE_C__DURATION__TYPE_SUPPORT_H_

#include <dds_dcps.h>

#if __cplusplus
extern "C"
{
#endif

#include <rosidl_generator_c/message_type_support.h>
#include <rosidl_typesupport_opensplice_c/visibility_control.h>

#include "builtin_interfaces/msg/duration__struct.h"

const rosidl_message_type_support_t *
  ROSIDL_GET_TYPE_SUPPORT_FUNCTION(builtin_interfaces, msg, Duration)();

#if __cplusplus
}
#endif

#endif  // ROSIDL_TYPESUPPORT_OPENSPLICE_C__DURATION__TYPE_SUPPORT_H_
