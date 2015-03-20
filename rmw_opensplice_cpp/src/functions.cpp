#include <iostream>
#include <stdexcept>

#include <ccpp_dds_dcps.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rosidl_generator_c/message_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/service_type_support.h>

// The extern "C" here enforces that overloading is not used.
extern "C"
{

RMW_PUBLIC const char * opensplice_cpp_identifier = "opensplice_static";

struct OpenSpliceStaticPublisherInfo
{
  DDS::DataWriter * topic_writer;
  const message_type_support_callbacks_t * callbacks;
};

struct OpenSpliceStaticSubscriberInfo
{
  DDS::DataReader * topic_reader;
  const message_type_support_callbacks_t * callbacks;
};

const char *
rmw_get_implementation_identifier()
{
  return opensplice_cpp_identifier;
}

rmw_ret_t
rmw_init()
{
  DDS::DomainParticipantFactory_var dp_factory = \
    DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory)
  {
    rmw_set_error_string("failed to get domain participant factory");
    return RMW_RET_ERROR;
  };
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name)
{
  DDS::DomainParticipantFactory_var dp_factory = \
    DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory)
  {
    rmw_set_error_string("failed to get domain participant factory");
    return nullptr;
  };

  // TODO: take the domain id from configuration
  DDS::DomainId_t domain = 0;

  DDS::DomainParticipant * participant = dp_factory->create_participant(
    domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE
  );
  if (!participant)
  {
    rmw_set_error_string("failed to create domain participant");
    return nullptr;
  };

  rmw_node_t * node = rmw_node_allocate();
  if (!node)
  {
    rmw_set_error_string("failed to allocate rmw_node_t");
    return nullptr;
  }
  node->implementation_identifier = opensplice_cpp_identifier;
  node->data = participant;
  return node;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node)
  {
    // Would not be able to teardown, so return error code
    rmw_set_error_string("received null pointer");
    return RMW_RET_ERROR;
  }
  // TODO: do DDS specific teardown
  rmw_node_free(node);
  return RMW_RET_OK;
}

rmw_publisher_t *
rmw_create_publisher(const rmw_node_t * node,
                     const rosidl_message_type_support_t * type_support,
                     const char * topic_name,
                     size_t queue_size)
{
  if (node->implementation_identifier != opensplice_cpp_identifier)
  {
    rmw_set_error_string("node does not share this implementation");
    // printf("node handle not from this implementation\n");
    // printf("but from: %s\n", node_handle._implementation_identifier);
    return nullptr;
  }

  DDS::DomainParticipant * participant = \
    static_cast<DDS::DomainParticipant *>(node->data);

  const message_type_support_callbacks_t * callbacks = \
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = std::string(callbacks->package_name) + \
                          "::dds_::" + callbacks->message_name + "_";

  callbacks->register_type(participant, type_name.c_str());

  DDS::PublisherQos publisher_qos;
  DDS::ReturnCode_t status;
  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("failed to get default publisher qos");
    // printf("get_default_publisher_qos() failed. Status = %d\n", status);
    return nullptr;
  };

  DDS::Publisher * dds_publisher = participant->create_publisher(
    publisher_qos, NULL, DDS::STATUS_MASK_NONE);
  if (!dds_publisher)
  {
    rmw_set_error_string("failed to create publisher");
    // printf("  create_publisher() could not create publisher\n");
    return nullptr;
  };

  DDS::TopicQos default_topic_qos;
  status = participant->get_default_topic_qos(default_topic_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("failed to get default topic qos");
    // printf("get_default_topic_qos() failed. Status = %d\n", status);
    return nullptr;
  };

  if (std::string(topic_name).find("/") != std::string::npos)
  {
    rmw_set_error_string("topic_name contains a '/'");
    return nullptr;
  }
  DDS::Topic * topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL,
    DDS::STATUS_MASK_NONE
  );
  if (!topic)
  {
    rmw_set_error_string("failed to create topic");
    // printf("  create_topic() could not create topic\n");
    return nullptr;
  };

  DDS::DataWriterQos default_datawriter_qos;
  status = dds_publisher->get_default_datawriter_qos(default_datawriter_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("failed to get default datawriter qos");
    // printf("get_default_datawriter_qos() failed. Status = %d\n", status);
    return nullptr;
  };

  DDS::DataWriter * topic_writer = dds_publisher->create_datawriter(
    topic, default_datawriter_qos, NULL, DDS::STATUS_MASK_NONE
  );
  if (!topic_writer)
  {
    rmw_set_error_string("failed to create datawriter");
    return nullptr;
  }

  OpenSpliceStaticPublisherInfo * publisher_info = \
    static_cast<OpenSpliceStaticPublisherInfo *>(rmw_allocate(
      sizeof(OpenSpliceStaticPublisherInfo)));
  publisher_info->topic_writer = topic_writer;
  publisher_info->callbacks = callbacks;

  rmw_publisher_t * publisher = rmw_publisher_allocate();
  if (!publisher)
  {
    rmw_set_error_string("failed to allocate rmw_publisher_t");
    return nullptr;
  }
  publisher->implementation_identifier = opensplice_cpp_identifier;
  publisher->data = publisher_info;

  return publisher;
}

rmw_ret_t
rmw_destroy_publisher(rmw_publisher_t * publisher)
{
  if (!publisher)
  {
    // Would not be able to teardown, so return error code
    rmw_set_error_string("received null pointer");
    return RMW_RET_ERROR;
  }
  // TODO: do DDS specific teardown
  rmw_publisher_free(publisher);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  if (publisher->implementation_identifier != opensplice_cpp_identifier)
  {
    rmw_set_error_string("publisher does not share this implementation");
    return RMW_RET_ERROR;
  }

  const OpenSpliceStaticPublisherInfo * publisher_info = \
    static_cast<const OpenSpliceStaticPublisherInfo *>(publisher->data);
  DDS::DataWriter * topic_writer = publisher_info->topic_writer;
  const message_type_support_callbacks_t * callbacks = \
    publisher_info->callbacks;

  callbacks->publish(topic_writer, ros_message);
  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(const rmw_node_t * node,
                        const rosidl_message_type_support_t * type_support,
                        const char * topic_name,
                        size_t queue_size)
{
  if (node->implementation_identifier != opensplice_cpp_identifier)
  {
    rmw_set_error_string("node does not share this implementation");
    return nullptr;
  }

  DDS::DomainParticipant * participant = \
    static_cast<DDS::DomainParticipant *>(node->data);

  const message_type_support_callbacks_t * callbacks = \
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = std::string(callbacks->package_name) + \
                          "::dds_::" + callbacks->message_name + "_";

  callbacks->register_type(participant, type_name.c_str());

  DDS::SubscriberQos subscriber_qos;
  DDS::ReturnCode_t status = \
    participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("failed to get default subscriber qos");
    // printf("get_default_subscriber_qos() failed. Status = %d\n", status);
    return nullptr;
  }

  DDS::Subscriber * dds_subscriber = participant->create_subscriber(
    subscriber_qos, NULL, DDS::STATUS_MASK_NONE
  );
  if (!dds_subscriber)
  {
    rmw_set_error_string("failed to create subscriber");
    // printf("  create_subscriber() could not create subscriber\n");
    return nullptr;
  }

  DDS::TopicQos default_topic_qos;
  status = participant->get_default_topic_qos(default_topic_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("faield to get default topic qos");
    // printf("get_default_topic_qos() failed. Status = %d\n", status);
    return nullptr;
  }

  DDS::Topic * topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL,
    DDS::STATUS_MASK_NONE
  );
  if (!topic)
  {
    rmw_set_error_string("failed to create topic");
    // printf("  create_topic() could not create topic\n");
    return nullptr;
  }

  DDS::DataReaderQos default_datareader_qos;
  status = dds_subscriber->get_default_datareader_qos(default_datareader_qos);
  if (status != DDS::RETCODE_OK)
  {
    rmw_set_error_string("failed to get default datareader qos");
    // printf("get_default_datareader_qos() failed. Status = %d\n", status);
    return nullptr;
  }

  DDS::DataReader * topic_reader = dds_subscriber->create_datareader(
    topic, default_datareader_qos, NULL, DDS::STATUS_MASK_NONE
  );

  OpenSpliceStaticSubscriberInfo * subscriber_info = \
    static_cast<OpenSpliceStaticSubscriberInfo *>(rmw_allocate(
      sizeof(OpenSpliceStaticSubscriberInfo)));
  subscriber_info->topic_reader = topic_reader;
  subscriber_info->callbacks = callbacks;

  rmw_subscription_t * subscription = rmw_subscription_allocate();
  if (!subscription)
  {
    rmw_set_error_string("failed to allocate rmw_subscription_t");
    return nullptr;
  }
  subscription->implementation_identifier = opensplice_cpp_identifier;
  subscription->data = subscriber_info;
  return subscription;
}

rmw_ret_t
rmw_destroy_subscription(rmw_subscription_t * subscription)
{
  if (!subscription)
  {
    // Would not be able to teardown, so return error code
    rmw_set_error_string("received null pointer");
    return RMW_RET_ERROR;
  }
  // TODO: do DDS specific teardown
  rmw_subscription_free(subscription);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  if (taken == NULL) {
    rmw_set_error_string("taken argument can't be null");
    return RMW_RET_ERROR;
  }

  if (subscription->implementation_identifier != opensplice_cpp_identifier)
  {
    rmw_set_error_string("subscription does not share this implementation");
    return RMW_RET_ERROR;
  }

  OpenSpliceStaticSubscriberInfo * subscriber_info = \
    static_cast<OpenSpliceStaticSubscriberInfo *>(subscription->data);
  DDS::DataReader * topic_reader = subscriber_info->topic_reader;
  const message_type_support_callbacks_t * callbacks = \
    subscriber_info->callbacks;

  *taken = callbacks->take(topic_reader, ros_message);

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  guard_condition->implementation_identifier = opensplice_cpp_identifier;
  guard_condition->data = static_cast<DDS::GuardCondition *>(
    rmw_allocate(sizeof(DDS::GuardCondition)));
  // Ensure constructor with "placement new"
  new (guard_condition->data) DDS::GuardCondition();
  return guard_condition;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition)
  {
    // Would not be able to teardown, so return error code
    rmw_set_error_string("received null pointer");
    return RMW_RET_ERROR;
  }
  // TODO: do DDS specific teardown
  // Explicitly call destructor since the "placement new" was used
  DDS::GuardCondition * dds_guard_condition = \
    static_cast<DDS::GuardCondition *>(guard_condition->data);
  dds_guard_condition->~GuardCondition();
  rmw_free(guard_condition->data);
  rmw_guard_condition_free(guard_condition);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition)
{
  if (guard_condition->implementation_identifier != opensplice_cpp_identifier)
  {
    rmw_set_error_string("guard_condition does not share this implementation");
    return RMW_RET_ERROR;
  }

  DDS::GuardCondition * dds_guard_condition = \
    static_cast<DDS::GuardCondition *>(guard_condition->data);
  dds_guard_condition->set_trigger_value(true);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_wait(rmw_subscriptions_t * subscriptions,
         rmw_guard_conditions_t * guard_conditions,
         rmw_services_t * services,
         rmw_clients_t * clients,
         bool non_blocking)
{
  // TODO(wjwwood): Handle services and service clients.
  DDS::WaitSet waitset;

  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i)
  {
    OpenSpliceStaticSubscriberInfo * subscriber_info = \
      static_cast<OpenSpliceStaticSubscriberInfo *>(
        subscriptions->subscribers[i]);
    DDS::DataReader * topic_reader = subscriber_info->topic_reader;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);
    waitset.attach_condition(condition);
  }

  // add a condition for each guard condition
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i)
  {
    DDS::GuardCondition * guard_condition = \
      static_cast<DDS::GuardCondition *>(
        guard_conditions->guard_conditions[i]);
    waitset.attach_condition(guard_condition);
  }

  // invoke wait until one of the conditions triggers
  DDS::ConditionSeq active_conditions;
  DDS::Duration_t timeout;
  timeout.sec = non_blocking ? 0 : 1;
  timeout.nanosec = 0;
  DDS::ReturnCode_t status = DDS::RETCODE_TIMEOUT;
  while (DDS::RETCODE_TIMEOUT == status)
  {
    status = waitset.wait(active_conditions, timeout);
    if (DDS::RETCODE_TIMEOUT == status)
    {
      if (non_blocking)
      {
        break;
      }
      continue;
    };
    if (status != DDS::RETCODE_OK)
    {
      rmw_set_error_string("failed to wait on waitset");
      // printf("wait() failed. Status = %d\n", status);
      return RMW_RET_ERROR;
    };
  }

  // set subscriber handles to zero for all not triggered status conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i)
  {
    OpenSpliceStaticSubscriberInfo * subscriber_info = \
      static_cast<OpenSpliceStaticSubscriberInfo *>(
        subscriptions->subscribers[i]);
    DDS::DataReader* topic_reader = subscriber_info->topic_reader;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition->get_trigger_value())
    {
      // if the status condition was not triggered
      // reset the subscriber handle
      subscriptions->subscribers[i] = 0;
    }
  }

  // set guard condition handles to zero for all not triggered guard conditions
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i)
  {
    DDS::GuardCondition * guard_condition = \
      static_cast<DDS::GuardCondition *>(
        guard_conditions->guard_conditions[i]);
    if (!guard_condition->get_trigger_value())
    {
      // if the guard condition was not triggered
      // reset the guard condition handle
      guard_conditions->guard_conditions[i] = 0;
    }
    else
    {
      // reset the trigger value
      guard_condition->set_trigger_value(false);
    }
  }
  return RMW_RET_OK;
}

// TODO(wjwwood): implement all of the service and service client functions.
rmw_client_t *
rmw_create_client(const rmw_node_t * node,
                  const rosidl_service_type_support_t * type_support,
                  const char * service_name)
{
  return NULL;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_request(const rmw_client_t * client, const void * ros_request,
                 int64_t * sequence_id)
{
  *sequence_id = -1;
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_take_response(const rmw_client_t * client,
                  void * ros_response, void * ros_request_header, bool * taken)
{
  if (taken == NULL) {
    rmw_set_error_string("taken argument can't be null");
    return RMW_RET_ERROR;
  }

  *taken = false;
  return RMW_RET_ERROR;
}

rmw_service_t *
rmw_create_service(const rmw_node_t * node,
                   const rosidl_service_type_support_t * type_support,
                   const char * service_name)
{
  return NULL;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(const rmw_service_t * service,
                 void * ros_request, void * ros_request_header, bool * taken)
{
  if (taken == NULL) {
    rmw_set_error_string("taken argument can't be null");
    return RMW_RET_ERROR;
  }

  *taken = false;
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_send_response(const rmw_service_t * service,
                  void * ros_request, void * ros_response)
{
  ros_request = nullptr;
  ros_response = nullptr;
  return RMW_RET_ERROR;
}

}  // extern "C"
