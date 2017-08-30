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

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <ccpp_dds_dcps.h>
#include <dds_dcps.h>

#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>

#include "rmw/types.h"

#include "rosidl_typesupport_opensplice_cpp/message_type_support.h"
#include "rosidl_typesupport_opensplice_cpp/service_type_support.h"

RMW_LOCAL
std::string
create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep);

RMW_LOCAL
std::string
get_ros_prefix_if_exists(const std::string & topic_name);

RMW_LOCAL
std::string
demangle_if_ros_topic(const std::string & topic_name);

RMW_LOCAL
std::string
demangle_if_ros_type(const std::string & dds_type_string);

// The extern "C" here enforces that overloading is not used.
extern "C"
{
class CustomDataReaderListener
  : public DDS::DataReaderListener
{
public:
  CustomDataReaderListener();

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

  void fill_topic_names_and_types(
    bool no_demangle,
    std::map<std::string, std::set<std::string>> & tnat);
  size_t count_topic(const char * topic_name);

  enum EndPointType
  {
    PublisherEP,
    SubscriberEP,
  };

protected:
  virtual void add_information(
    const DDS::SampleInfo & sample_info,
    const std::string & topic_name,
    const std::string & type_name,
    EndPointType end_point_type);
  virtual void remove_information(
    const DDS::SampleInfo & sample_info,
    EndPointType end_point_type);

  std::mutex mutex_;

private:
  struct TopicDescriptor
  {
    DDS::InstanceHandle_t instance_handle;
    std::string name;
    std::string type;
  };
  std::map<std::string, std::multiset<std::string>> topic_names_and_types_;
  std::list<TopicDescriptor> topic_descriptors_;
  bool print_discovery_logging_;
};

class CustomPublisherListener
  : public CustomDataReaderListener
{
public:
  explicit CustomPublisherListener(rmw_guard_condition_t * graph_guard_condition);
  virtual void on_data_available(DDS::DataReader * reader);

private:
  rmw_guard_condition_t * graph_guard_condition_;
};

class CustomSubscriberListener
  : public CustomDataReaderListener
{
public:
  explicit CustomSubscriberListener(rmw_guard_condition_t * graph_guard_condition);
  virtual void on_data_available(DDS::DataReader * reader);

private:
  rmw_guard_condition_t * graph_guard_condition_;
};

struct OpenSpliceStaticNodeInfo
{
  DDS::DomainParticipant * participant;
  rmw_guard_condition_t * graph_guard_condition;
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

struct OpenSpliceWaitSetInfo
{
  DDS::WaitSet * waitset;
  DDS::ConditionSeq * active_conditions;
  DDS::ConditionSeq * attached_conditions;
};
}  // extern "C"

#endif  // TYPES_HPP_
