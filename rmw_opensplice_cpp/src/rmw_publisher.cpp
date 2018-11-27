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

#ifdef __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
#include <ccpp_dds_dcps.h>
#ifdef __clang__
# pragma GCC diagnostic pop
#endif
#include <dds_dcps.h>
#include <string>

#include "rosidl_typesupport_opensplice_c/identifier.h"
#include "rosidl_typesupport_opensplice_cpp/identifier.hpp"
#include "rosidl_typesupport_opensplice_cpp/misc.hpp"
#include "rosidl_typesupport_opensplice_cpp/impl/error_checking.hpp"

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "identifier.hpp"
#include "qos.hpp"
#include "types.hpp"
#include "typesupport_macros.hpp"

using rosidl_typesupport_opensplice_cpp::impl::check_get_default_publisher_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_get_default_topic_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_datawriter;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_publisher;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_topic;
using rosidl_typesupport_opensplice_cpp::process_topic_name;

// The extern "C" here enforces that overloading is not used.
extern "C"
{
rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_supports,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  RMW_OPENSPLICE_EXTRACT_MESSAGE_TYPESUPPORT(
    type_supports, type_support)

  if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("publisher topic is null or empty string");
    return nullptr;
  }

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  std::string type_name = create_type_name(callbacks, "msg");

  const char * error_string = callbacks->register_type(participant, type_name.c_str());
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to register the type: ") + error_string).c_str());
    return nullptr;
  }

  DDS::PublisherQos publisher_qos;
  DDS::ReturnCode_t status;
  status = participant->get_default_publisher_qos(publisher_qos);
  if (nullptr != check_get_default_publisher_qos(status)) {
    RMW_SET_ERROR_MSG(check_get_default_publisher_qos(status));
    return nullptr;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_publisher_t * publisher = nullptr;
  DDS::Publisher * dds_publisher = nullptr;
  DDS::TopicQos default_topic_qos;
  DDS::Topic * topic = nullptr;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataWriter * topic_writer = nullptr;
  OpenSpliceStaticPublisherInfo * publisher_info = nullptr;
  std::string topic_str;

  // Begin initializing elements.
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_publisher_t");
    goto fail;
  }

  if (!process_topic_name(
      topic_name, qos_profile->avoid_ros_namespace_conventions, topic_str))
  {
    RMW_SET_ERROR_MSG("failed to process topic name");
    goto fail;
  }

  dds_publisher = participant->create_publisher(publisher_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!dds_publisher) {
    RMW_SET_ERROR_MSG("failed to create publisher");
    goto fail;
  }

  status = participant->get_default_topic_qos(default_topic_qos);
  if (nullptr != check_get_default_topic_qos(status)) {
    RMW_SET_ERROR_MSG(check_get_default_topic_qos(status));
    goto fail;
  }

  topic = participant->create_topic(
    topic_str.c_str(), type_name.c_str(), default_topic_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic) {
    RMW_SET_ERROR_MSG("failed to create topic");
    goto fail;
  }

  if (!get_datawriter_qos(dds_publisher, *qos_profile, datawriter_qos)) {
    goto fail;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("failed to create datawriter");
    goto fail;
  }

  publisher_info = static_cast<OpenSpliceStaticPublisherInfo *>(
    rmw_allocate(sizeof(OpenSpliceStaticPublisherInfo)));
  publisher_info->dds_topic = topic;
  publisher_info->dds_publisher = dds_publisher;
  publisher_info->topic_writer = topic_writer;
  publisher_info->callbacks = callbacks;
  static_assert(
    sizeof(OpenSplicePublisherGID) <= RMW_GID_STORAGE_SIZE,
    "RMW_GID_STORAGE_SIZE insufficient to store the rmw_opensplice_cpp GID implemenation."
  );
  // Zero the data memory.
  memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  {
    auto publisher_gid =
      reinterpret_cast<OpenSplicePublisherGID *>(publisher_info->publisher_gid.data);
    publisher_gid->publication_handle = topic_writer->get_instance_handle();
  }
  publisher_info->publisher_gid.implementation_identifier = opensplice_cpp_identifier;

  publisher->implementation_identifier = opensplice_cpp_identifier;
  publisher->data = publisher_info;
  publisher->topic_name = reinterpret_cast<const char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(publisher->topic_name), topic_name, strlen(topic_name) + 1);

  return publisher;
fail:
  if (publisher) {
    rmw_publisher_free(publisher);
  }
  if (dds_publisher) {
    if (topic_writer) {
      status = dds_publisher->delete_datawriter(topic_writer);
      if (nullptr != check_delete_datawriter(status)) {
        fprintf(stderr, "%s\n", check_delete_datawriter(status));
      }
    }
    status = participant->delete_publisher(dds_publisher);
    if (nullptr != check_delete_publisher(status)) {
      fprintf(stderr, "%s\n", check_delete_publisher(status));
    }
  }
  if (topic) {
    status = participant->delete_topic(topic);
    if (nullptr != check_delete_topic(status)) {
      fprintf(stderr, "%s\n", check_delete_topic(status));
    }
  }
  if (publisher_info) {
    rmw_free(publisher_info);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!publisher) {
    RMW_SET_ERROR_MSG("pointer handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

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
  auto result = RMW_RET_OK;
  OpenSpliceStaticPublisherInfo * publisher_info =
    static_cast<OpenSpliceStaticPublisherInfo *>(publisher->data);
  if (publisher_info) {
    DDS::Publisher * dds_publisher = publisher_info->dds_publisher;
    if (dds_publisher) {
      DDS::DataWriter * topic_writer = publisher_info->topic_writer;
      if (topic_writer) {
        DDS::ReturnCode_t status = dds_publisher->delete_datawriter(topic_writer);
        if (nullptr != check_delete_datawriter(status)) {
          RMW_SET_ERROR_MSG(check_delete_datawriter(status));
          result = RMW_RET_ERROR;
        }
      }
      DDS::ReturnCode_t status = participant->delete_publisher(dds_publisher);
      if (nullptr != check_delete_publisher(status)) {
        RMW_SET_ERROR_MSG(check_delete_publisher(status));
        result = RMW_RET_ERROR;
      }
    }
    DDS::Topic * topic = publisher_info->dds_topic;
    if (topic) {
      DDS::ReturnCode_t status = participant->delete_topic(topic);
      if (nullptr != check_delete_topic(status)) {
        fprintf(stderr, "%s\n", check_delete_topic(status));
        result = RMW_RET_ERROR;
      }
    }
    rmw_free(publisher_info);
  }
  if (publisher->topic_name) {
    rmw_free(const_cast<char *>(publisher->topic_name));
  }
  rmw_publisher_free(publisher);
  return result;
}
}  // extern "C"
