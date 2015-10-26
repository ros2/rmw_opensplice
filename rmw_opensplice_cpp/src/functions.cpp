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

#include <cassert>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <stdlib.h>  // For _putenv_s.
#endif

#include <ccpp_dds_dcps.h>
#include <dds_dcps.h>
#include <u_instanceHandle.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rosidl_generator_c/message_type_support.h"
#include "rosidl_generator_c/service_type_support.h"
#include "rosidl_typesupport_opensplice_cpp/identifier.hpp"
#include "rosidl_typesupport_opensplice_cpp/impl/error_checking.hpp"
#include "rosidl_typesupport_opensplice_cpp/message_type_support.h"
#include "rosidl_typesupport_opensplice_cpp/service_type_support.h"

inline std::string
_create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return std::string(callbacks->package_name) +
         "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

using rosidl_typesupport_opensplice_cpp::impl::check_get_default_datareader_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_get_default_datawriter_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_get_default_publisher_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_get_default_topic_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_datareader;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_datawriter;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_publisher;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_subscriber;
using rosidl_typesupport_opensplice_cpp::impl::check_delete_topic;

template<typename DDSEntityQos>
bool set_entity_qos_from_profile(const rmw_qos_profile_t & qos_profile,
  DDSEntityQos & entity_qos)
{
  switch (qos_profile.history) {
    case RMW_QOS_POLICY_KEEP_LAST_HISTORY:
      entity_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_KEEP_ALL_HISTORY:
      entity_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABLE:
      entity_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile.durability) {
    case RMW_QOS_POLICY_TRANSIENT_LOCAL_DURABILITY:
      entity_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_VOLATILE_DURABILITY:
      entity_qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth =
      static_cast<DDS::Long>(qos_profile.depth);
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth
  )
  {
    if (qos_profile.depth > (std::numeric_limits<DDS::Long>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<DDS::Long>(qos_profile.depth);
  }

  return true;
}

// The extern "C" here enforces that overloading is not used.
extern "C"
{
using rosidl_typesupport_opensplice_cpp::typesupport_opensplice_identifier;
const char * opensplice_cpp_identifier = "opensplice_static";

class CustomDataReaderListener
  : public DDS::DataReaderListener
{
public:
  void on_requested_deadline_missed(
    DDS::DataReader_ptr, const DDS::RequestedDeadlineMissedStatus &)
  {}
  void on_requested_incompatible_qos(
    DDS::DataReader_ptr, const DDS::RequestedIncompatibleQosStatus &)
  {}
  void on_sample_rejected(
    DDS::DataReader_ptr, const DDS::SampleRejectedStatus &)
  {}
  void on_liveliness_changed(
    DDS::DataReader_ptr, const DDS::LivelinessChangedStatus &)
  {}
  void on_data_available(
    DDS::DataReader_ptr)
  {}
  void on_subscription_matched(
    DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus &)
  {}
  void on_sample_lost(
    DDS::DataReader_ptr, const DDS::SampleLostStatus &)
  {}
  std::map<std::string, std::multiset<std::string>> topic_names_and_types;

protected:
  virtual void add_information(
    const DDS::SampleInfo & sample_info,
    const std::string & topic_name,
    const std::string & type_name)
  {
    // store topic name and type name
    auto & topic_types = topic_names_and_types[topic_name];
    topic_types.insert(type_name);
    // store mapping to instance handle
    TopicDescriptor topic_descriptor;
    topic_descriptor.instance_handle = sample_info.instance_handle;
    topic_descriptor.name = topic_name;
    topic_descriptor.type = type_name;
    topic_descriptors.push_back(topic_descriptor);
  }
  virtual void remove_information(const DDS::SampleInfo & sample_info)
  {
    // find entry by instance handle
    for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
      if (it->instance_handle == sample_info.instance_handle) {
        // remove entries
        auto & topic_types = topic_names_and_types[it->name];
        topic_types.erase(topic_types.find(it->type));
        if (topic_types.empty()) {
          topic_names_and_types.erase(it->name);
        }
        topic_descriptors.erase(it);
        break;
      }
    }
  }

private:
  struct TopicDescriptor
  {
    DDS::InstanceHandle_t instance_handle;
    std::string name;
    std::string type;
  };
  std::list<TopicDescriptor> topic_descriptors;
};

class CustomPublisherListener
  : public CustomDataReaderListener
{
public:
  virtual void on_data_available(DDS::DataReader * reader)
  {
    DDS::PublicationBuiltinTopicDataDataReader * builtin_reader =
      DDS::PublicationBuiltinTopicDataDataReader::_narrow(reader);

    DDS::PublicationBuiltinTopicDataSeq data_seq;
    DDS::SampleInfoSeq info_seq;
    DDS::ReturnCode_t retcode = builtin_reader->take(
      data_seq, info_seq, DDS::LENGTH_UNLIMITED,
      DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);

    if (retcode == DDS::RETCODE_NO_DATA) {
      return;
    }
    if (retcode != DDS::RETCODE_OK) {
      fprintf(stderr, "failed to access data from the built-in reader\n");
      return;
    }

    for (DDS::ULong i = 0; i < data_seq.length(); ++i) {
      if (info_seq[i].valid_data) {
        add_information(info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in());
      } else {
        remove_information(info_seq[i]);
      }
    }

    builtin_reader->return_loan(data_seq, info_seq);
  }
};

class CustomSubscriberListener
  : public CustomDataReaderListener
{
public:
  virtual void on_data_available(DDS::DataReader * reader)
  {
    DDS::SubscriptionBuiltinTopicDataDataReader * builtin_reader =
      DDS::SubscriptionBuiltinTopicDataDataReader::_narrow(reader);

    DDS::SubscriptionBuiltinTopicDataSeq data_seq;
    DDS::SampleInfoSeq info_seq;
    DDS::ReturnCode_t retcode = builtin_reader->take(
      data_seq, info_seq, DDS::LENGTH_UNLIMITED,
      DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);

    if (retcode == DDS::RETCODE_NO_DATA) {
      return;
    }
    if (retcode != DDS::RETCODE_OK) {
      fprintf(stderr, "failed to access data from the built-in reader\n");
      return;
    }

    for (DDS::ULong i = 0; i < data_seq.length(); ++i) {
      if (info_seq[i].valid_data) {
        add_information(info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in());
      } else {
        remove_information(info_seq[i]);
      }
    }

    builtin_reader->return_loan(data_seq, info_seq);
  }
};

struct OpenSpliceStaticNodeInfo
{
  DDS::DomainParticipant * participant;
  CustomPublisherListener * publisher_listener;
  CustomSubscriberListener * subscriber_listener;
};

typedef struct OpenSplicePublisherGID
{
  DDS::InstanceHandle_t publication_handle;
} OpenSplicePublisherGID;

struct OpenSpliceStaticPublisherInfo
{
  DDS::Topic * dds_topic;
  DDS::Publisher * dds_publisher;
  DDS::DataWriter * topic_writer;
  const message_type_support_callbacks_t * callbacks;
  rmw_gid_t publisher_gid;
};

struct OpenSpliceStaticSubscriberInfo
{
  DDS::Topic * dds_topic;
  DDS::Subscriber * dds_subscriber;
  DDS::DataReader * topic_reader;
  DDS::ReadCondition * read_condition;
  const message_type_support_callbacks_t * callbacks;
  bool ignore_local_publications;
};

struct OpenSpliceStaticClientInfo
{
  void * requester_;
  DDS::DataReader * response_datareader_;
  DDS::ReadCondition * read_condition_;
  const service_type_support_callbacks_t * callbacks_;
};

struct OpenSpliceStaticServiceInfo
{
  void * responder_;
  DDS::DataReader * request_datareader_;
  DDS::ReadCondition * read_condition_;
  const service_type_support_callbacks_t * callbacks_;
};

rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode)
{
  if (retcode == DDS::RETCODE_OK) {
    return RMW_RET_OK;
  }
  if (retcode == DDS::RETCODE_OUT_OF_RESOURCES) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: out of resources");
  } else if (retcode == DDS::RETCODE_BAD_PARAMETER) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: condition pointer was invalid");
  } else {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset");
  }
  return RMW_RET_ERROR;
}

const char *
rmw_get_implementation_identifier()
{
  return opensplice_cpp_identifier;
}

bool get_datareader_qos(
  DDS::Subscriber * subscriber,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataReaderQos & datareader_qos)
{
  if (subscriber == nullptr) {
    datareader_qos = DATAREADER_QOS_DEFAULT;
  } else {
    DDS::ReturnCode_t status;
    status = subscriber->get_default_datareader_qos(datareader_qos);
    if (nullptr != (check_get_default_datareader_qos(status))) {
      RMW_SET_ERROR_MSG(check_get_default_datareader_qos(status));
      return false;
    }
  }
  return set_entity_qos_from_profile(qos_profile, datareader_qos);
}

bool get_datawriter_qos(
  DDS::Publisher * publisher,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataWriterQos & datawriter_qos)
{
  if (publisher == nullptr) {
    datawriter_qos = DATAWRITER_QOS_DEFAULT;
  } else {
    DDS::ReturnCode_t status;
    status = publisher->get_default_datawriter_qos(datawriter_qos);
    if (nullptr != check_get_default_datawriter_qos(status)) {
      RMW_SET_ERROR_MSG(check_get_default_datawriter_qos(status));
      return false;
    }
  }

  return set_entity_qos_from_profile(qos_profile, datawriter_qos);
}

rmw_ret_t
rmw_init(const char * qos_xml_filename)
{
  std::string qos_xml_filename_str(qos_xml_filename);
  if (!qos_xml_filename_str.empty()) {
    RMW_SET_ERROR_MSG("rmw_opensplice does not support passing QoS profiles via XML files");
    return RMW_RET_ERROR;
  }

  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    RMW_SET_ERROR_MSG("failed to get domain participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name, size_t domain_id)
{
  (void)name;

  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    RMW_SET_ERROR_MSG("failed to get domain participant factory");
    return nullptr;
  }

  DDS::DomainId_t domain = static_cast<DDS::DomainId_t>(domain_id);
  DDS::DomainParticipant * participant = nullptr;

  // Make sure that the OSPL_URI is set, otherwise node creation will fail.
  char * ospl_uri = nullptr;
  const char * ospl_uri_env = "OSPL_URI";
#ifndef _WIN32
  ospl_uri = getenv(ospl_uri_env);
#else
  size_t ospl_uri_size;
  _dupenv_s(&ospl_uri, &ospl_uri_size, ospl_uri_env);
#endif
  if (!ospl_uri) {
    RMW_SET_ERROR_MSG("OSPL_URI not set");
    return nullptr;
  } else {
#ifdef _WIN32
    free(ospl_uri);
#endif
  }

  // Ensure the ROS_DOMAIN_ID env variable is set, otherwise parsing of the config may fail.
  // Also make sure the it is set to the domain_id passed in, otherwise it will fail.
  // But first backup the current ROS_DOMAIN_ID.
  char * ros_domain_id = nullptr;
  const char * env_var = "ROS_DOMAIN_ID";
#ifndef _WIN32
  ros_domain_id = getenv(env_var);
#else
  size_t ros_domain_id_size;
  _dupenv_s(&ros_domain_id, &ros_domain_id_size, env_var);
#endif

  // On Windows, setting the ROS_DOMAIN_ID does not fix the problem, so error early.
#ifdef _WIN32
  if (!ros_domain_id) {
    RMW_SET_ERROR_MSG("environment variable ROS_DOMAIN_ID is not set");
    fprintf(stderr, "[rmw_opensplice_cpp]: error: %s\n", rmw_get_error_string_safe());
    return nullptr;
  }
#endif

  // Set the ROS_DOMAIN_ID explicitly (if not Windows).
#ifndef _WIN32
  auto domain_id_as_string = std::to_string(domain_id);
  int ret = 0;
  ret = setenv(env_var, domain_id_as_string.c_str(), 1);
  if (0 != ret) {
    RMW_SET_ERROR_MSG("failed to set the ROS_DOMAIN_ID");
    return nullptr;
  }
#endif

  participant = dp_factory->create_participant(
    domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create domain participant");
    return NULL;
  }

  // Restore the ROS_DOMAIN_ID if necessary (and not Windows).
#ifndef _WIN32
  if (ros_domain_id) {
    ret = setenv(env_var, ros_domain_id, 1);
    if (0 != ret) {
      RMW_SET_ERROR_MSG("failed to reset the ROS_DOMAIN_ID");
      return nullptr;
    }
  } else {
    // Otherwise unset it again.
    ret = unsetenv(env_var);
    if (0 != ret) {
      RMW_SET_ERROR_MSG("failed to unset the ROS_DOMAIN_ID");
      return nullptr;
    }
  }
#endif

  rmw_node_t * node = nullptr;
  OpenSpliceStaticNodeInfo * node_info = nullptr;
  CustomPublisherListener * publisher_listener = nullptr;
  CustomSubscriberListener * subscriber_listener = nullptr;
  void * buf = nullptr;

  DDS::DataReader * data_reader = nullptr;
  DDS::PublicationBuiltinTopicDataDataReader * builtin_publication_datareader = nullptr;
  DDS::SubscriptionBuiltinTopicDataDataReader * builtin_subscription_datareader = nullptr;
  DDS::Subscriber * builtin_subscriber = participant->get_builtin_subscriber();
  if (!builtin_subscriber) {
    RMW_SET_ERROR_MSG("builtin subscriber handle is null");
    goto fail;
  }

  // setup publisher listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSPublication");
  builtin_publication_datareader =
    DDS::PublicationBuiltinTopicDataDataReader::_narrow(data_reader);
  if (!builtin_publication_datareader) {
    RMW_SET_ERROR_MSG("builtin publication datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(publisher_listener, buf, goto fail, CustomPublisherListener)
  buf = nullptr;
  builtin_publication_datareader->set_listener(publisher_listener, DDS::DATA_AVAILABLE_STATUS);

  data_reader = builtin_subscriber->lookup_datareader("DCPSSubscription");
  builtin_subscription_datareader =
    DDS::SubscriptionBuiltinTopicDataDataReader::_narrow(data_reader);
  if (!builtin_subscription_datareader) {
    RMW_SET_ERROR_MSG("builtin subscription datareader handle is null");
    goto fail;
  }

  // setup subscriber listener
  buf = rmw_allocate(sizeof(CustomSubscriberListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(subscriber_listener, buf, goto fail, CustomSubscriberListener)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS::DATA_AVAILABLE_STATUS);

  node = rmw_node_allocate();
  if (!node) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
    goto fail;
  }

  buf = rmw_allocate(sizeof(OpenSpliceStaticNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, OpenSpliceStaticNodeInfo)
  buf = nullptr;
  node_info->participant = participant;
  node_info->publisher_listener = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;

  node->implementation_identifier = opensplice_cpp_identifier;
  node->data = node_info;

  return node;
fail:
  if (participant) {
    if (dp_factory->delete_participant(participant) != DDS::RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking domain participant while handling failure at: " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(publisher_listener);
  }
  if (subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(subscriber_listener);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~OpenSpliceStaticNodeInfo(), OpenSpliceStaticNodeInfo)
    rmw_free(node_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  if (node) {
    rmw_node_free(node);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("received null pointer");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    RMW_SET_ERROR_MSG("failed to get domain participant factory");
    return RMW_RET_ERROR;
  }
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
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  if (participant->delete_contained_entities() != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete contained entities of participant");
    result = RMW_RET_ERROR;
  }

  if (dp_factory->delete_participant(participant) != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
    result = RMW_RET_ERROR;
  }

  if (node_info->publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(node_info->publisher_listener);
    node_info->publisher_listener = nullptr;
  }
  if (node_info->subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(node_info->subscriber_listener);
    node_info->subscriber_listener = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;
  rmw_node_free(node);
  return result;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t & qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

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
  std::string type_name = _create_type_name(callbacks, "msg");

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
  // Begin initializing elements.
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_publisher_t");
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

  if (std::string(topic_name).find("/") != std::string::npos) {
    RMW_SET_ERROR_MSG("topic_name contains a '/'");
    goto fail;
  }
  topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic) {
    RMW_SET_ERROR_MSG("failed to create topic");
    goto fail;
  }

  if (!get_datawriter_qos(dds_publisher, qos_profile, datawriter_qos)) {
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
  rmw_publisher_free(publisher);
  return result;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)
  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }

  const OpenSpliceStaticPublisherInfo * publisher_info =
    static_cast<const OpenSpliceStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataWriter * topic_writer = publisher_info->topic_writer;
  const message_type_support_callbacks_t * callbacks = publisher_info->callbacks;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  const char * error_string = callbacks->publish(topic_writer, ros_message);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to publish:") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t & qos_profile,
  bool ignore_local_publications)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

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
  std::string type_name = _create_type_name(callbacks, "msg");

  const char * error_string = callbacks->register_type(participant, type_name.c_str());
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to register the type: ") + error_string).c_str());
    return nullptr;
  }

  DDS::SubscriberQos subscriber_qos;
  DDS::ReturnCode_t status = participant->get_default_subscriber_qos(subscriber_qos);
  if (nullptr != check_get_default_datareader_qos(status)) {
    RMW_SET_ERROR_MSG(check_get_default_datareader_qos(status));
    return nullptr;
  }

  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  DDS::Subscriber * dds_subscriber = nullptr;
  DDS::TopicQos default_topic_qos;
  DDS::Topic * topic = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataReader * topic_reader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * buf = nullptr;
  OpenSpliceStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_subscription_t");
    goto fail;
  }
  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!dds_subscriber) {
    RMW_SET_ERROR_MSG("failed to create subscriber");
    goto fail;
  }

  status = participant->get_default_topic_qos(default_topic_qos);
  if (nullptr != check_get_default_topic_qos(status)) {
    RMW_SET_ERROR_MSG(check_get_default_topic_qos(status));
    goto fail;
  }

  topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic) {
    RMW_SET_ERROR_MSG("failed to create topic");
    goto fail;
  }

  if (!get_datareader_qos(dds_subscriber, qos_profile, datareader_qos)) {
    goto fail;
  }

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("failed to create topic reader");
    goto fail;
  }

  read_condition = topic_reader->create_readcondition(
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  // Allocate memory for the OpenSpliceStaticSubscriberInfo object.
  buf = rmw_allocate(sizeof(OpenSpliceStaticSubscriberInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the instance in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(subscriber_info, buf, goto fail, OpenSpliceStaticSubscriberInfo)
  buf = nullptr;  // Only free the subscriber_info pointer; don't need the buf pointer anymore.
  subscriber_info->dds_topic = topic;
  subscriber_info->dds_subscriber = dds_subscriber;
  subscriber_info->topic_reader = topic_reader;
  subscriber_info->read_condition = read_condition;
  subscriber_info->callbacks = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = opensplice_cpp_identifier;
  subscription->data = subscriber_info;
  return subscription;
fail:
  if (dds_subscriber) {
    if (topic_reader) {
      if (read_condition) {
        if (topic_reader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
          fprintf(stderr, "leaking readcondition while handling failure\n");
        }
      }
      status = dds_subscriber->delete_datareader(topic_reader);
      if (nullptr != check_delete_datareader(status)) {
        fprintf(stderr, "%s\n", check_delete_datareader(status));
      }
      status = participant->delete_subscriber(dds_subscriber);
      if (nullptr != check_delete_subscriber(status)) {
        fprintf(stderr, "%s\n", check_delete_subscriber(status));
      }
    }
  }
  if (topic) {
    status = participant->delete_topic(topic);
    if (nullptr != check_delete_topic(status)) {
      fprintf(stderr, "%s\n", check_delete_topic(status));
    }
  }
  if (subscriber_info) {
    rmw_free(subscriber_info);
  }
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  if (buf) {
    rmw_free(buf);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, opensplice_cpp_identifier,
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
  OpenSpliceStaticSubscriberInfo * subscription_info =
    static_cast<OpenSpliceStaticSubscriberInfo *>(subscription->data);
  if (subscription_info) {
    DDS::Subscriber * dds_subscriber = subscription_info->dds_subscriber;
    if (dds_subscriber) {
      DDS::DataReader * topic_reader = subscription_info->topic_reader;
      if (topic_reader) {
        DDS::ReadCondition * read_condition = subscription_info->read_condition;
        if (read_condition) {
          if (topic_reader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
            RMW_SET_ERROR_MSG("failed to delete readcondition");
            result = RMW_RET_ERROR;
          }
          subscription_info->read_condition = nullptr;
        }
        if (topic_reader->delete_contained_entities() != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete contained entities of datareader");
          result = RMW_RET_ERROR;
        }
        DDS::ReturnCode_t status = dds_subscriber->delete_datareader(topic_reader);
        if (nullptr != check_delete_datareader(status)) {
          RMW_SET_ERROR_MSG(check_delete_datareader(status));
          result = RMW_RET_ERROR;
        }
      }
      DDS::ReturnCode_t status = participant->delete_subscriber(dds_subscriber);
      if (nullptr != check_delete_subscriber(status)) {
        RMW_SET_ERROR_MSG(check_delete_subscriber(status));
        result = RMW_RET_ERROR;
      }
    }
    DDS::Topic * topic = subscription_info->dds_topic;
    if (topic) {
      DDS::ReturnCode_t status = participant->delete_topic(topic);
      if (nullptr != check_delete_topic(status)) {
        fprintf(stderr, "%s\n", check_delete_topic(status));
        result = RMW_RET_ERROR;
      }
    }
    rmw_free(subscription_info);
  }
  rmw_subscription_free(subscription);
  return result;
}

RMW_LOCAL
rmw_ret_t
_take(
  const rmw_subscription_t * subscription, void * ros_message, bool * taken,
  DDS::InstanceHandle_t * sending_publication_handle)
{
  // Note: gid is allowed to be nullptr, if unused.
  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (ros_message == nullptr) {
    RMW_SET_ERROR_MSG("ros_message argument cannot be null");
    return RMW_RET_ERROR;
  }

  if (taken == nullptr) {
    RMW_SET_ERROR_MSG("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticSubscriberInfo * subscriber_info =
    static_cast<OpenSpliceStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataReader * topic_reader = subscriber_info->topic_reader;
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("topic reader handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  const char * error_string = callbacks->take(
    topic_reader,
    subscriber_info->ignore_local_publications,
    ros_message, taken, sending_publication_handle);
  // If no data was taken, that's not captured as an error here, but instead taken is set to false.
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to take: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  return _take(subscription, ros_message, taken, nullptr);
}

rmw_ret_t
rmw_take_with_info(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_message_info_t * message_info)
{
  if (!message_info) {
    RMW_SET_ERROR_MSG("message info is null");
    return RMW_RET_ERROR;
  }
  DDS::InstanceHandle_t sending_publication_handle;
  auto ret = _take(subscription, ros_message, taken, &sending_publication_handle);
  if (ret != RMW_RET_OK) {
    // Error string is already set.
    return RMW_RET_ERROR;
  }

  rmw_gid_t * sender_gid = &message_info->publisher_gid;
  sender_gid->implementation_identifier = opensplice_cpp_identifier;
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);
  auto detail = reinterpret_cast<OpenSplicePublisherGID *>(sender_gid->data);
  detail->publication_handle = sending_publication_handle;

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("failed to allocate guard condition");
    goto fail;
  }
  guard_condition->implementation_identifier = opensplice_cpp_identifier;
  guard_condition->data = rmw_allocate(sizeof(DDS::GuardCondition));
  if (!guard_condition->data) {
    RMW_SET_ERROR_MSG("failed to allocate dds guard condition");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(
    guard_condition->data, guard_condition->data, goto fail, DDS::GuardCondition)
  return guard_condition;
fail:
  if (guard_condition->data) {
    // The allocation succeeded but the constructor threw, so deallocate.
    rmw_free(guard_condition->data);
  }
  if (guard_condition) {
    rmw_guard_condition_free(guard_condition);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  DDS::GuardCondition * dds_guard_condition =
    static_cast<DDS::GuardCondition *>(guard_condition->data);
  // Explicitly call destructor since the "placement new" was used
  RMW_TRY_DESTRUCTOR(
    dds_guard_condition->~GuardCondition(), GuardCondition, result = RMW_RET_ERROR)
  rmw_free(guard_condition->data);
  rmw_guard_condition_free(guard_condition);
  return result;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::GuardCondition * dds_guard_condition =
    static_cast<DDS::GuardCondition *>(guard_condition->data);
  if (!dds_guard_condition) {
    RMW_SET_ERROR_MSG("guard condition is null");
    return RMW_RET_ERROR;
  }
  if (dds_guard_condition->set_trigger_value(true) != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to set trigger value to true");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_time_t * wait_timeout)
{
  DDS::WaitSet waitset;

  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    OpenSpliceStaticSubscriberInfo * subscriber_info =
      static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = subscriber_info->read_condition;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t status = check_attach_condition_error(
      waitset.attach_condition(read_condition));
    if (status != RMW_RET_OK) {
      return status;
    }
  }

  // add a condition for each guard condition
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDS::GuardCondition * guard_condition =
      static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition) {
      RMW_SET_ERROR_MSG("guard condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t status = check_attach_condition_error(
      waitset.attach_condition(guard_condition));
    if (status != RMW_RET_OK) {
      return status;
    }
  }

  // add a condition for each service
  for (size_t i = 0; i < services->service_count; ++i) {
    OpenSpliceStaticServiceInfo * service_info =
      static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = service_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t status = check_attach_condition_error(
      waitset.attach_condition(read_condition));
    if (status != RMW_RET_OK) {
      return status;
    }
  }

  // add a condition for each client
  for (size_t i = 0; i < clients->client_count; ++i) {
    OpenSpliceStaticClientInfo * client_info =
      static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = client_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t status = check_attach_condition_error(
      waitset.attach_condition(read_condition));
    if (status != RMW_RET_OK) {
      return status;
    }
  }

  // invoke wait until one of the conditions triggers
  DDS::ConditionSeq active_conditions;
  DDS::Duration_t timeout;
  if (!wait_timeout) {
    timeout = DDS::DURATION_INFINITE;
  } else {
    timeout.sec = static_cast<DDS::Long>(wait_timeout->sec);
    timeout.nanosec = static_cast<DDS::Long>(wait_timeout->nsec);
  }
  DDS::ReturnCode_t status = waitset.wait(active_conditions, timeout);

  if (status == DDS::RETCODE_TIMEOUT) {
    return RMW_RET_TIMEOUT;
  }

  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to wait on waitset");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered status conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    OpenSpliceStaticSubscriberInfo * subscriber_info =
      static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = subscriber_info->read_condition;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    if (!read_condition->get_trigger_value()) {
      // if the status condition was not triggered
      // reset the subscriber handle
      subscriptions->subscribers[i] = 0;
    }
  }

  // set guard condition handles to zero for all not triggered guard conditions
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDS::GuardCondition * guard_condition =
      static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition) {
      RMW_SET_ERROR_MSG("guard condition handle is null");
      return RMW_RET_ERROR;
    }

    if (!guard_condition->get_trigger_value()) {
      // if the guard condition was not triggered
      // reset the guard condition handle
      guard_conditions->guard_conditions[i] = 0;
    } else {
      // reset the trigger value
      if (guard_condition->set_trigger_value(false) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to set trigger value to false");
        return RMW_RET_ERROR;
      }
    }
  }

  // set service handles to zero for all not triggered conditions
  for (size_t i = 0; i < services->service_count; ++i) {
    OpenSpliceStaticServiceInfo * service_info =
      static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = service_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == read_condition) {
        break;
      }
    }
    // if service condition is not found in the active set
    // reset the service handle
    if (!(j < active_conditions.length())) {
      services->services[i] = 0;
    }
  }

  // set client handles to zero for all not triggered conditions
  for (size_t i = 0; i < clients->client_count; ++i) {
    OpenSpliceStaticClientInfo * client_info =
      static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = client_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == read_condition) {
        break;
      }
    }
    // if client condition is not found in the active set
    // reset the client handle
    if (!(j < active_conditions.length())) {
      clients->clients[i] = 0;
    }
  }
  return RMW_RET_OK;
}

rmw_client_t *
rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t & qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

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

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }

  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;

  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  const char * error_string = nullptr;
  DDS::DataReader * response_datareader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  OpenSpliceStaticClientInfo * client_info = nullptr;
  // Begin initializing elements.
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    goto fail;
  }

  if (!get_datareader_qos(nullptr, qos_profile, datareader_qos)) {
    goto fail;
  }

  if (!get_datawriter_qos(nullptr, qos_profile, datawriter_qos)) {
    goto fail;
  }

  error_string = callbacks->create_requester(
    participant, service_name,
    reinterpret_cast<void **>(&requester),
    reinterpret_cast<void **>(&response_datareader),
    reinterpret_cast<const void *>(&datareader_qos),
    reinterpret_cast<const void *>(&datawriter_qos),
    &rmw_allocate);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to create requester: ") + error_string).c_str());
    goto fail;
  }
  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester: requester is null");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("failed to create requester: response_datareader is null");
    goto fail;
  }

  read_condition = response_datareader->create_readcondition(
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  client_info = static_cast<OpenSpliceStaticClientInfo *>(
    rmw_allocate(sizeof(OpenSpliceStaticClientInfo)));
  if (!client_info) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = opensplice_cpp_identifier;
  client->data = client_info;
  return client;
fail:
  if (response_datareader) {
    if (read_condition) {
      if (response_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
        fprintf(stderr, "leaking readcondition while handling failure\n");
      }
    }
  }

  if (requester) {
    const char * error_string = callbacks->destroy_requester(requester, &rmw_free);
    if (error_string) {
      std::stringstream ss;
      ss << "failed to destroy requester: " << error_string << ", at: " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (client_info) {
    rmw_free(client_info);
  }
  if (client) {
    rmw_client_free(client);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);

  auto result = RMW_RET_OK;
  if (client_info) {
    auto response_datareader = client_info->response_datareader_;
    if (response_datareader) {
      auto read_condition = client_info->read_condition_;
      if (read_condition) {
        if (response_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
        client_info->read_condition_ = nullptr;
      }
    }
  } else {
    RMW_SET_ERROR_MSG("client_info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(client_info->callbacks_);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  const char * error_string = callbacks->destroy_requester(client_info->requester_, &rmw_free);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to destroy requester: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }

  rmw_free(client_info);
  rmw_client_free(client);
  return result;
}

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client, const void * ros_request,
  int64_t * sequence_id)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  const char * error_string = callbacks->send_request(requester, ros_request, sequence_id);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to send request: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(const rmw_client_t * client, void * ros_request_header,
  void * ros_response, bool * taken)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }
  if (taken == nullptr) {
    RMW_SET_ERROR_MSG("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  const char * error_string =
    callbacks->take_response(requester, ros_request_header, ros_response, taken);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to take response: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t & qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

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

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;

  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_service_t * service = nullptr;
  const char * error_string = nullptr;
  DDS::DataReader * request_datareader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * responder = nullptr;
  OpenSpliceStaticServiceInfo * service_info = nullptr;
  // Begin initialization of elements.
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("failed to allocate service");
    goto fail;
  }

  if (!get_datareader_qos(nullptr, qos_profile, datareader_qos)) {
    goto fail;
  }

  if (!get_datawriter_qos(nullptr, qos_profile, datawriter_qos)) {
    goto fail;
  }

  error_string = callbacks->create_responder(
    participant, service_name,
    reinterpret_cast<void **>(&responder),
    reinterpret_cast<void **>(&request_datareader),
    reinterpret_cast<const void *>(&datareader_qos),
    reinterpret_cast<const void *>(&datawriter_qos),
    &rmw_allocate);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to create responder: ") + error_string).c_str());
    goto fail;
  }

  read_condition = request_datareader->create_readcondition(
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(rmw_allocate(sizeof(OpenSpliceStaticServiceInfo)));
  if (!service_info) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  service_info->responder_ = responder;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;
  service_info->read_condition_ = read_condition;

  service->implementation_identifier = opensplice_cpp_identifier;
  service->data = service_info;
  return service;
fail:
  if (request_datareader) {
    if (read_condition) {
      if (request_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
        fprintf(stderr, "leaking readcondition while handling failure\n");
      }
    }
  }

  if (responder) {
    const char * error_string = callbacks->destroy_responder(responder, &rmw_free);
    if (error_string) {
      std::stringstream ss;
      ss << "failed to destroy responder: " << error_string << ", at: " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (service_info) {
    rmw_free(service_info);
  }
  if (service) {
    rmw_service_free(service);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);

  auto result = RMW_RET_OK;
  if (service_info) {
    auto request_datareader = service_info->request_datareader_;
    if (request_datareader) {
      auto read_condition = service_info->read_condition_;
      if (read_condition) {
        if (request_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
        service_info->read_condition_ = nullptr;
      }
    }
  } else {
    RMW_SET_ERROR_MSG("service_info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(service_info->callbacks_);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  const char * error_string = callbacks->destroy_responder(service_info->responder_, &rmw_free);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to destroy responder: ") + error_string).c_str());
  }

  rmw_free(service_info);
  rmw_service_free(service);
  return result;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  void * ros_request_header, void * ros_request, bool * taken)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }
  if (taken == nullptr) {
    RMW_SET_ERROR_MSG("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }
  void * responder = service_info->responder_;
  if (!responder) {
    RMW_SET_ERROR_MSG("responder handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  const char * error_string =
    callbacks->take_request(responder, ros_request_header, ros_request, taken);
  if (error_string) {
    RMW_SET_ERROR_MSG((std::string("failed to take request: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_response(
  const rmw_service_t * service,
  void * ros_request_header, void * ros_response)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }
  void * responder = service_info->responder_;
  if (!responder) {
    RMW_SET_ERROR_MSG("responder handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  const char * error_string =
    callbacks->send_response(responder, ros_request_header, ros_response);
  if (error_string) {
    RMW_SET_ERROR_MSG(error_string);
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

void
destroy_topic_names_and_types(
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (topic_names_and_types->topic_count) {
    for (size_t i = 0; i < topic_names_and_types->topic_count; ++i) {
      delete topic_names_and_types->topic_names[i];
      delete topic_names_and_types->type_names[i];
      topic_names_and_types->topic_names[i] = nullptr;
      topic_names_and_types->type_names[i] = nullptr;
    }
    if (topic_names_and_types->topic_names) {
      rmw_free(topic_names_and_types->topic_names);
      topic_names_and_types->topic_names = nullptr;
    }
    if (topic_names_and_types->type_names) {
      rmw_free(topic_names_and_types->type_names);
      topic_names_and_types->type_names = nullptr;
    }
    topic_names_and_types->topic_count = 0;
  }
}

rmw_ret_t
rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != opensplice_cpp_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_names_and_types) {
    RMW_SET_ERROR_MSG("topics handle is null");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->topic_count) {
    RMW_SET_ERROR_MSG("topic count is not zero");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->topic_names) {
    RMW_SET_ERROR_MSG("topic names is not null");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->type_names) {
    RMW_SET_ERROR_MSG("type names is not null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> topics_with_multiple_types;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }

  // ignore inconsistent types
  std::map<std::string, std::string> topics;
  for (auto & it : topics_with_multiple_types) {
    if (it.second.size() != 1) {
      fprintf(stderr, "topic type mismatch - ignoring topic '%s'\n", it.first.c_str());
      continue;
    }
    topics[it.first] = *it.second.begin();
  }

  // reformat type name
  std::string substr = "::msg::dds_::";
  for (auto & it : topics) {
    size_t substr_pos = it.second.find(substr);
    if (it.second[it.second.size() - 1] == '_' && substr_pos != std::string::npos) {
      it.second = it.second.substr(0, substr_pos) + "/" + it.second.substr(
        substr_pos + substr.size(), it.second.size() - substr_pos - substr.size() - 1);
    }
  }

  // copy data into result handle
  if (topics.size() > 0) {
    topic_names_and_types->topic_names = static_cast<char **>(
      rmw_allocate(sizeof(char *) * topics.size()));
    if (!topic_names_and_types->topic_names) {
      RMW_SET_ERROR_MSG("failed to allocate memory for topic names")
      return RMW_RET_ERROR;
    }
    topic_names_and_types->type_names = static_cast<char **>(
      rmw_allocate(sizeof(char *) * topics.size()));
    if (!topic_names_and_types->type_names) {
      rmw_free(topic_names_and_types->topic_names);
      RMW_SET_ERROR_MSG("failed to allocate memory for type names")
      return RMW_RET_ERROR;
    }
    for (auto it : topics) {
      char * topic_name = strdup(it.first.c_str());
      if (!topic_name) {
        RMW_SET_ERROR_MSG("failed to allocate memory for topic name")
        goto fail;
      }
      char * type_name = strdup(it.second.c_str());
      if (!type_name) {
        rmw_free(topic_name);
        RMW_SET_ERROR_MSG("failed to allocate memory for type name")
        goto fail;
      }
      size_t i = topic_names_and_types->topic_count;
      topic_names_and_types->topic_names[i] = topic_name;
      topic_names_and_types->type_names[i] = type_name;
      ++topic_names_and_types->topic_count;
    }
  }

  return RMW_RET_OK;
fail:
  destroy_topic_names_and_types(topic_names_and_types);
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_destroy_topic_names_and_types(
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (!topic_names_and_types) {
    RMW_SET_ERROR_MSG("topics handle is null");
    return RMW_RET_ERROR;
  }
  destroy_topic_names_and_types(topic_names_and_types);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_publishers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != opensplice_cpp_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->publisher_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_subscribers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != opensplice_cpp_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->subscriber_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_gid_for_publisher(const rmw_publisher_t * publisher, rmw_gid_t * gid)
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier,
    opensplice_cpp_identifier,
    return RMW_RET_ERROR)
  if (!gid) {
    RMW_SET_ERROR_MSG("gid is null");
    return RMW_RET_ERROR;
  }

  const OpenSpliceStaticPublisherInfo * publisher_info =
    static_cast<const OpenSpliceStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }

  *gid = publisher_info->publisher_gid;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_compare_gids_equal(const rmw_gid_t * gid1, const rmw_gid_t * gid2, bool * result)
{
  if (!gid1) {
    RMW_SET_ERROR_MSG("gid1 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid1,
    gid1->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)
  if (!gid2) {
    RMW_SET_ERROR_MSG("gid2 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid2,
    gid2->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)
  if (!result) {
    RMW_SET_ERROR_MSG("result is null");
    return RMW_RET_ERROR;
  }
  auto detail1 = reinterpret_cast<const OpenSplicePublisherGID *>(gid1->data);
  if (!detail1) {
    RMW_SET_ERROR_MSG("gid1 is invalid");
    return RMW_RET_ERROR;
  }
  auto detail2 = reinterpret_cast<const OpenSplicePublisherGID *>(gid2->data);
  if (!detail2) {
    RMW_SET_ERROR_MSG("gid2 is invalid");
    return RMW_RET_ERROR;
  }
  *result = detail1->publication_handle == detail2->publication_handle;
  return RMW_RET_OK;
}
}  // extern "C"
