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

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <ccpp_dds_dcps.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rmw/rmw.h>
#include <rosidl_generator_c/message_type_support.h>
#include <rosidl_generator_c/service_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/identifier.hpp>
#include <rosidl_typesupport_opensplice_cpp/impl/error_checking.hpp>
#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/service_type_support.h>

#include <rmw/impl/cpp/macros.hpp>

inline std::string
_create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return std::string(callbacks->package_name) +
         "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

using namespace rosidl_typesupport_opensplice_cpp::impl;

// The extern "C" here enforces that overloading is not used.
extern "C"
{

using rosidl_typesupport_opensplice_cpp::typesupport_opensplice_identifier;
const char * opensplice_cpp_identifier = "opensplice_static";

struct OpenSpliceStaticPublisherInfo
{
  DDS::Topic * dds_topic;
  DDS::Publisher * dds_publisher;
  DDS::DataWriter * topic_writer;
  const message_type_support_callbacks_t * callbacks;
};

struct OpenSpliceStaticSubscriberInfo
{
  DDS::Topic * dds_topic;
  DDS::Subscriber * dds_subscriber;
  DDS::DataReader * topic_reader;
  const message_type_support_callbacks_t * callbacks;
  bool ignore_local_publications;
};

struct OpenSpliceStaticClientInfo
{
  void * requester_;
  DDS::DataReader * response_datareader_;
  const service_type_support_callbacks_t * callbacks_;
};

struct OpenSpliceStaticServiceInfo
{
  void * responder_;
  DDS::DataReader * request_datareader_;
  const service_type_support_callbacks_t * callbacks_;
};

const char *
rmw_get_implementation_identifier()
{
  return opensplice_cpp_identifier;
}

rmw_ret_t
rmw_init()
{
  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    rmw_set_error_string("failed to get domain participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name)
{
  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    rmw_set_error_string("failed to get domain participant factory");
    return nullptr;
  }

  // TODO: take the domain id from configuration
  DDS::DomainId_t domain = 0;
  DDS::DomainParticipant * participant = nullptr;

  rmw_node_t * node = rmw_node_allocate();
  if (!node) {
    rmw_set_error_string("failed to allocate rmw_node_t");
    goto fail;
  }

  participant = dp_factory->create_participant(
    domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
  if (!participant) {
    rmw_set_error_string("failed to create domain participant");
    goto fail;
  }

  node->implementation_identifier = opensplice_cpp_identifier;
  node->data = participant;

  return node;
fail:
  if (node) {
    rmw_node_free(node);
  }
  if (participant) {
    if (dp_factory->delete_participant(participant) != DDS::RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking domain participant while handling failure at: " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node) {
    rmw_set_error_string("received null pointer");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    rmw_set_error_string("failed to get domain participant factory");
    return RMW_RET_ERROR;
  }
  auto result = RMW_RET_OK;
  if (node->data) {
    if (dp_factory->delete_participant((DDS::DomainParticipant *)node->data) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to delete participant");
      result = RMW_RET_ERROR;
    }
  }
  rmw_node_free(node);
  return result;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks, "msg");

  const char * error_string = callbacks->register_type(participant, type_name.c_str());
  if (error_string) {
    rmw_set_error_string((std::string("failed to register the type: ") + error_string).c_str());
    return nullptr;
  }

  DDS::PublisherQos publisher_qos;
  DDS::ReturnCode_t status;
  status = participant->get_default_publisher_qos(publisher_qos);
  if (nullptr != check_get_default_publisher_qos(status)) {
    rmw_set_error_string(check_get_default_publisher_qos(status));
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
    rmw_set_error_string("failed to allocate rmw_publisher_t");
    goto fail;
  }
  dds_publisher = participant->create_publisher(publisher_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!dds_publisher) {
    rmw_set_error_string("failed to create publisher");
    goto fail;
  }

  status = participant->get_default_topic_qos(default_topic_qos);
  if (nullptr != check_get_default_topic_qos(status)) {
    rmw_set_error_string(check_get_default_topic_qos(status));
    goto fail;
  }

  if (std::string(topic_name).find("/") != std::string::npos) {
    rmw_set_error_string("topic_name contains a '/'");
    goto fail;
  }
  topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic) {
    rmw_set_error_string("failed to create topic");
    goto fail;
  }

  status = dds_publisher->get_default_datawriter_qos(datawriter_qos);
  if (nullptr != check_get_default_datawriter_qos(status)) {
    rmw_set_error_string(check_get_default_datawriter_qos(status));
    goto fail;
  }

  // ensure the history depth is at least the requested queue size
  // *INDENT-OFF*
  if (
    datawriter_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    datawriter_qos.history.depth < queue_size)
  // *INDENT-ON*
  {
    datawriter_qos.history.depth = queue_size;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic_writer) {
    rmw_set_error_string("failed to create datawriter");
    goto fail;
  }

  publisher_info = static_cast<OpenSpliceStaticPublisherInfo *>(
    rmw_allocate(sizeof(OpenSpliceStaticPublisherInfo)));
  publisher_info->dds_topic = topic;
  publisher_info->dds_publisher = dds_publisher;
  publisher_info->topic_writer = topic_writer;
  publisher_info->callbacks = callbacks;

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
    rmw_set_error_string("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!publisher) {
    rmw_set_error_string("pointer handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);
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
          rmw_set_error_string(check_delete_datawriter(status));
          result = RMW_RET_ERROR;
        }
      }
      DDS::ReturnCode_t status = participant->delete_publisher(dds_publisher);
      if (nullptr != check_delete_publisher(status)) {
        rmw_set_error_string(check_delete_publisher(status));
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
    rmw_set_error_string("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  const OpenSpliceStaticPublisherInfo * publisher_info =
    static_cast<const OpenSpliceStaticPublisherInfo *>(publisher->data);
  DDS::DataWriter * topic_writer = publisher_info->topic_writer;
  const message_type_support_callbacks_t * callbacks = publisher_info->callbacks;

  const char * error_string = callbacks->publish(topic_writer, ros_message);
  if (error_string) {
    rmw_set_error_string((std::string("failed to publish:") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size,
  bool ignore_local_publications)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return NULL)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks, "msg");

  const char * error_string = callbacks->register_type(participant, type_name.c_str());
  if (error_string) {
    rmw_set_error_string((std::string("failed to register the type: ") + error_string).c_str());
    return nullptr;
  }

  DDS::SubscriberQos subscriber_qos;
  DDS::ReturnCode_t status = participant->get_default_subscriber_qos(subscriber_qos);
  if (nullptr != check_get_default_datareader_qos(status)) {
    rmw_set_error_string(check_get_default_datareader_qos(status));
    return nullptr;
  }

  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  DDS::Subscriber * dds_subscriber = nullptr;
  DDS::TopicQos default_topic_qos;
  DDS::Topic * topic = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataReader * topic_reader = nullptr;
  OpenSpliceStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    rmw_set_error_string("failed to allocate rmw_subscription_t");
    goto fail;
  }
  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!dds_subscriber) {
    rmw_set_error_string("failed to create subscriber");
    goto fail;
  }

  status = participant->get_default_topic_qos(default_topic_qos);
  if (nullptr != check_get_default_topic_qos(status)) {
    rmw_set_error_string(check_get_default_topic_qos(status));
    goto fail;
  }

  topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!topic) {
    rmw_set_error_string("failed to create topic");
    goto fail;
  }

  status = dds_subscriber->get_default_datareader_qos(datareader_qos);
  if (nullptr != check_get_default_datareader_qos(status)) {
    rmw_set_error_string(check_get_default_datareader_qos(status));
    goto fail;
  }

  // ensure the history depth is at least the requested queue size
  // *INDENT-OFF*
  if (
    datareader_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    datareader_qos.history.depth < queue_size)
  // *INDENT-ON*
  {
    datareader_qos.history.depth = queue_size;
  }

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos, NULL, DDS::STATUS_MASK_NONE);

  subscriber_info = static_cast<OpenSpliceStaticSubscriberInfo *>(
    rmw_allocate(sizeof(OpenSpliceStaticSubscriberInfo)));
  subscriber_info->dds_topic = topic;
  subscriber_info->dds_subscriber = dds_subscriber;
  subscriber_info->topic_reader = topic_reader;
  subscriber_info->callbacks = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = opensplice_cpp_identifier;
  subscription->data = subscriber_info;
  return subscription;
fail:
  if (dds_subscriber) {
    if (topic_reader) {
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
  return nullptr;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    rmw_set_error_string("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);
  auto result = RMW_RET_OK;
  OpenSpliceStaticSubscriberInfo * subscription_info =
    static_cast<OpenSpliceStaticSubscriberInfo *>(subscription->data);
  if (subscription_info) {
    DDS::Subscriber * dds_subscriber = subscription_info->dds_subscriber;
    if (dds_subscriber) {
      DDS::DataReader * topic_reader = subscription_info->topic_reader;
      if (topic_reader) {
        if (topic_reader->delete_contained_entities() != DDS::RETCODE_OK) {
          rmw_set_error_string("failed to delete contained entities of datareader");
          result = RMW_RET_ERROR;
        }
        DDS::ReturnCode_t status = dds_subscriber->delete_datareader(topic_reader);
        if (nullptr != check_delete_datareader(status)) {
          rmw_set_error_string(check_delete_datareader(status));
          result = RMW_RET_ERROR;
        }
      }
      DDS::ReturnCode_t status = participant->delete_subscriber(dds_subscriber);
      if (nullptr != check_delete_subscriber(status)) {
        rmw_set_error_string(check_delete_subscriber(status));
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

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  if (!subscription) {
    rmw_set_error_string("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (ros_message == nullptr) {
    rmw_set_error_string("ros_message argument cannot be null");
    return RMW_RET_ERROR;
  }

  if (taken == nullptr) {
    rmw_set_error_string("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticSubscriberInfo * subscriber_info =
    static_cast<OpenSpliceStaticSubscriberInfo *>(subscription->data);
  DDS::DataReader * topic_reader = subscriber_info->topic_reader;
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks;

  const char * error_string = callbacks->take(
    topic_reader,
    subscriber_info->ignore_local_publications,
    ros_message, taken);
  // If no data was taken, that's not capture as an error here, but instead taken is set to false.
  if (error_string) {
    rmw_set_error_string((std::string("failed to take: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    rmw_set_error_string("failed to allocate guard condition");
    goto fail;
  }
  guard_condition->implementation_identifier = opensplice_cpp_identifier;
  guard_condition->data = rmw_allocate(sizeof(DDS::GuardCondition));
  if (!guard_condition->data) {
    rmw_set_error_string("failed to allocate dds guard condition");
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
    rmw_set_error_string("guard condition handle is null");
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
    rmw_set_error_string("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::GuardCondition * dds_guard_condition =
    static_cast<DDS::GuardCondition *>(guard_condition->data);
  if (dds_guard_condition->set_trigger_value(true) != DDS::RETCODE_OK) {
    rmw_set_error_string("failed to set trigger value to true");
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
  bool non_blocking)
{
  DDS::WaitSet waitset;

  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    OpenSpliceStaticSubscriberInfo * subscriber_info =
      static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    DDS::DataReader * topic_reader = subscriber_info->topic_reader;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from datareader");
      return RMW_RET_ERROR;
    }
    if (condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses on condition");
      return RMW_RET_ERROR;
    }
    if (waitset.attach_condition(condition) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to attach condition to waitset");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each guard condition
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDS::GuardCondition * guard_condition =
      static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
    if (waitset.attach_condition(guard_condition) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to attach condition to waitset");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each service
  for (unsigned long i = 0; i < services->service_count; ++i) {
    OpenSpliceStaticServiceInfo * service_info =
      static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
    DDS::DataReader * request_datareader = service_info->request_datareader_;
    DDS::StatusCondition * condition = request_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from request datareader");
      return RMW_RET_ERROR;
    }
    if (condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses on condition");
      return RMW_RET_ERROR;
    }
    if (waitset.attach_condition(condition) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to attach condition to waitset");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each client
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    OpenSpliceStaticClientInfo * client_info =
      static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
    DDS::DataReader * response_datareader = client_info->response_datareader_;
    DDS::StatusCondition * condition = response_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from response datareader");
      return RMW_RET_ERROR;
    }
    if (condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses on condition");
      return RMW_RET_ERROR;
    }
    if (waitset.attach_condition(condition) != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to attach condition to waitset");
      return RMW_RET_ERROR;
    }
  }

  // invoke wait until one of the conditions triggers
  DDS::ConditionSeq active_conditions;
  DDS::Duration_t timeout;
  timeout.sec = non_blocking ? 0 : 1;
  timeout.nanosec = 0;
  DDS::ReturnCode_t status = DDS::RETCODE_TIMEOUT;
  while (DDS::RETCODE_TIMEOUT == status) {
    status = waitset.wait(active_conditions, timeout);
    if (DDS::RETCODE_TIMEOUT == status) {
      if (non_blocking) {
        break;
      }
      continue;
    }
    if (status != DDS::RETCODE_OK) {
      rmw_set_error_string("failed to wait on waitset");
      return RMW_RET_ERROR;
    }
  }

  // set subscriber handles to zero for all not triggered status conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    OpenSpliceStaticSubscriberInfo * subscriber_info =
      static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    DDS::DataReader * topic_reader = subscriber_info->topic_reader;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from datareader");
      return RMW_RET_ERROR;
    }
    if (!condition->get_trigger_value()) {
      // if the status condition was not triggered
      // reset the subscriber handle
      subscriptions->subscribers[i] = 0;
    }
  }

  // set guard condition handles to zero for all not triggered guard conditions
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDS::GuardCondition * guard_condition =
      static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition->get_trigger_value()) {
      // if the guard condition was not triggered
      // reset the guard condition handle
      guard_conditions->guard_conditions[i] = 0;
    } else {
      // reset the trigger value
      if (guard_condition->set_trigger_value(false) != DDS::RETCODE_OK) {
        rmw_set_error_string("failed to set trigger value to false");
        return RMW_RET_ERROR;
      }
    }
  }

  // set service handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < services->service_count; ++i) {
    OpenSpliceStaticServiceInfo * service_info =
      static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
    DDS::DataReader * request_datareader = service_info->request_datareader_;
    DDS::StatusCondition * condition = request_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from request datareader");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
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
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    OpenSpliceStaticClientInfo * client_info =
      static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
    DDS::DataReader * response_datareader = client_info->response_datareader_;
    DDS::StatusCondition * condition = response_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("failed to get status condition from response datareader");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
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
  const char * service_name)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  const char * error_string = nullptr;
  DDS::DataReader * response_datareader = nullptr;
  void * requester = nullptr;
  OpenSpliceStaticClientInfo * client_info = nullptr;
  // Begin initializing elements.
  client = rmw_client_allocate();
  if (!client) {
    rmw_set_error_string("failed to allocate client");
    goto fail;
  }
  error_string = callbacks->create_requester(
    participant, service_name,
    reinterpret_cast<void **>(&requester),
    reinterpret_cast<void **>(&response_datareader),
    &rmw_allocate);
  if (error_string) {
    rmw_set_error_string((std::string("failed to create requester: ") + error_string).c_str());
    goto fail;
  }
  if (!requester) {
    rmw_set_error_string("failed to create requester: requester is null");
    goto fail;
  }
  if (!response_datareader) {
    rmw_set_error_string("failed to create requester: response_datareader is null");
    goto fail;
  }

  client_info = static_cast<OpenSpliceStaticClientInfo *>(
    rmw_allocate(sizeof(OpenSpliceStaticClientInfo)));
  if (!client_info) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;

  client->implementation_identifier = opensplice_cpp_identifier;
  client->data = client_info;
  return client;
fail:
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
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);
  if (!client_info) {
    rmw_set_error_string("client_info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(client_info->callbacks_);
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  const char * error_string = callbacks->destroy_requester(client_info->requester_, &rmw_free);
  if (error_string) {
    rmw_set_error_string((std::string("failed to destroy requester: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }

  rmw_free(client_info);
  rmw_client_free(client);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client, const void * ros_request,
  int64_t * sequence_id)
{
  if (!client) {
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);
  void * requester = client_info->requester_;
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  const char * error_string = callbacks->send_request(requester, ros_request, sequence_id);
  if (error_string) {
    rmw_set_error_string((std::string("failed to send request: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(const rmw_client_t * client, void * ros_request_header,
  void * ros_response, bool * taken)
{
  if (!client) {
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (taken == nullptr) {
    rmw_set_error_string("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticClientInfo * client_info =
    static_cast<OpenSpliceStaticClientInfo *>(client->data);
  void * requester = client_info->requester_;
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  const char * error_string =
    callbacks->take_response(requester, ros_request_header, ros_response, taken);
  if (error_string) {
    rmw_set_error_string((std::string("failed to take response: ") + error_string).c_str());
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return nullptr)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support,
    type_support->typesupport_identifier, typesupport_opensplice_identifier,
    return nullptr)

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(node->data);

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_service_t * service = nullptr;
  const char * error_string = nullptr;
  DDS::DataReader * request_datareader = nullptr;
  void * responder = nullptr;
  OpenSpliceStaticServiceInfo * service_info = nullptr;
  // Begin initialization of elements.
  service = rmw_service_allocate();
  if (!service) {
    rmw_set_error_string("failed to allocate service");
    goto fail;
  }
  error_string = callbacks->create_responder(
    participant, service_name,
    reinterpret_cast<void **>(&responder),
    reinterpret_cast<void **>(&request_datareader),
    &rmw_allocate);
  if (error_string) {
    rmw_set_error_string((std::string("failed to create responder: ") + error_string).c_str());
    goto fail;
  }

  service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(rmw_allocate(sizeof(OpenSpliceStaticServiceInfo)));
  if (!service_info) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  service_info->responder_ = responder;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;

  service->implementation_identifier = opensplice_cpp_identifier;
  service->data = service_info;
  return service;
fail:
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
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);
  if (!service_info) {
    rmw_set_error_string("service info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(service_info->callbacks_);
  const char * error_string = callbacks->destroy_responder(service_info->responder_, &rmw_free);
  if (error_string) {
    rmw_set_error_string((std::string("failed to destroy responder: ") + error_string).c_str());
  }

  rmw_free(service_info);
  rmw_service_free(service);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  void * ros_request_header, void * ros_request, bool * taken)
{
  if (!service) {
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  if (taken == nullptr) {
    rmw_set_error_string("taken argument cannot be null");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);
  void * responder = service_info->responder_;
  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  const char * error_string =
    callbacks->take_request(responder, ros_request_header, ros_request, taken);
  if (error_string) {
    rmw_set_error_string((std::string("failed to take request: ") + error_string).c_str());
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
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  OpenSpliceStaticServiceInfo * service_info =
    static_cast<OpenSpliceStaticServiceInfo *>(service->data);
  if (!service_info) {
    rmw_set_error_string("service info handle is null");
    return RMW_RET_ERROR;
  }
  void * responder = service_info->responder_;
  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  const char * error_string =
    callbacks->send_response(responder, ros_request_header, ros_response);
  if (error_string) {
    rmw_set_error_string(error_string);
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

}  // extern "C"
