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

#include <vector>
#include <string>

#include "rcutils/types/string_array.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw/sanity_checks.h"
#include "types.hpp"

// The extern "C" here enforces that overloading is not used.
extern "C"
{
rmw_ret_t
rmw_get_node_names(
  const rmw_node_t * node,
  rcutils_string_array_t * node_names)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_names) != RMW_RET_OK) {
    return RMW_RET_ERROR;
  }

  // Lookup the ROS Node names from the DDS Builtin Participant Topic userData
  //   For extensibility and backwards compatibility a format is applied that must be understood.
  //   The format specifies a list of name-value pairs,
  //   Format (byte ascii string) syntax : { <name> '=' <value> ';' }*
  //   A <name> is a string and a <value> is a sequence of octets.
  //   The '=' and ';' characters are reserved delimiters.
  //   For <name> only alphanumeric character are allowed.
  //   For <value> there is no limitation, ';' can be used as part of the <value> when escaped by
  //   a second ';'.
  //   Implemented policy value: "name=<node-name>;"
  //   userData not following this policy will be ignored completely.

  // Lookup all Builtin Participant Topic DataReader instance handles to access the userData.
  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  DDS::InstanceHandleSeq handles;
  if (participant->get_discovered_participants(handles) != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }

  // Collect all Node names from the list of instances

  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // allocate a temporary list for all Node names according to the maximum that can be expected.
  int length = handles.length();
  rcutils_string_array_t list = rcutils_get_zero_initialized_string_array();
  rcutils_ret_t rcutils_ret = rcutils_string_array_init(&list, length, &allocator);
  int n = 0;
  for (auto i = 0; i < length; ++i) {
    DDS::ParticipantBuiltinTopicData pbtd;

    auto dds_ret = participant->get_discovered_participant_data(pbtd, handles[i]);
    if (dds_ret == DDS::RETCODE_OK) {
      uint8_t * buf = pbtd.user_data.value.get_buffer(false);
      if (buf) {
        std::vector<uint8_t> kv(buf, buf + pbtd.user_data.value.length());
        auto map = rmw::impl::cpp::parse_key_value(kv);
        auto found = map.find("name");
        if (found != map.end()) {
          std::string name(found->second.begin(), found->second.end());
          list.data[n] = rcutils_strndup(name.c_str(), name.size(), allocator);
          if (!list.data[n]) {
            RMW_SET_ERROR_MSG("could not allocate memory for node name")
            rcutils_ret = rcutils_string_array_fini(&list);
            if (rcutils_ret != RCUTILS_RET_OK) {
              RCUTILS_LOG_ERROR_NAMED(
                "rmw_opensplice_cpp",
                "failed to cleanup during error handling: %s", rcutils_get_error_string_safe())
            }
            return RMW_RET_BAD_ALLOC;
          }
          n++;
        }
      }
    } else {
      RMW_SET_ERROR_MSG("unable to fetch discovered participants data.");
      return RMW_RET_ERROR;
    }
  }

  // Allocate the node_names out-buffer according to the number of Node names
  rcutils_ret = rcutils_string_array_init(node_names, n, &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    rmw_ret_t retcode = rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    rcutils_ret = rcutils_string_array_fini(&list);
    if (rcutils_ret != RCUTILS_RET_OK) {
      RCUTILS_LOG_ERROR_NAMED(
        "rmw_opensplice_cpp",
        "failed to cleanup during error handling: %s", rcutils_get_error_string_safe())
    }
    return retcode;
  }

  // Move the content from temporary list to the out-buffer and release temporary list.
  for (auto i = 0; i < n; ++i) {
    node_names->data[i] = list.data[i];
    list.data[i] = NULL;
  }
  rcutils_ret = rcutils_string_array_fini(&list);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RCUTILS_LOG_ERROR_NAMED(
      "rmw_opensplice_cpp",
      "failed to cleanup during error handling: %s", rcutils_get_error_string_safe())
  }
  return RMW_RET_OK;
}
}  // extern "C"
