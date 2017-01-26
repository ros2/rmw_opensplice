// Copyright 2014-2015 Open Source Robotics Foundation, Inc.
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

#include <map>
#include <set>
#include <string>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/sanity_checks.h"
#include "rmw/types.h"

#include "identifier.hpp"
#include "types.hpp"

// The extern "C" here enforces that overloading is not used.
extern "C"
{
rmw_ret_t
rmw_get_node_names(
  const rmw_node_t * node,
  rmw_string_array_t * node_names)
{
  RMW_SET_ERROR_MSG("get_node_names is not supported for Opensplice");
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_destroy_node_names(
  rmw_string_array_t * node_names)
{
  RMW_SET_ERROR_MSG("get_node_names is not supported for Opensplice");
  return RMW_RET_ERROR;
}
}  // extern "C"
