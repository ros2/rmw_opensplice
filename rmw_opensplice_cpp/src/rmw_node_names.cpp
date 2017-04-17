// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "c_utilities/types.h"

// TODO(karsten1987): Implement based on
// https://github.com/PrismTech/opensplice/blob/master/docs/pdf/OpenSplice_refman_CPP.pdf

// The extern "C" here enforces that overloading is not used.
extern "C"
{
rmw_ret_t
rmw_get_node_names(
  const rmw_node_t * /* node */,
  utilities_string_array_t * /* node_names */)
{
  RMW_SET_ERROR_MSG("get_node_names is not supported for Opensplice");
  return RMW_RET_ERROR;
}

}  // extern "C"
