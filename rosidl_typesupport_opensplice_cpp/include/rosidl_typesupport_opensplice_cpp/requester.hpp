// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef __rosidl_typesupport_opensplice_cpp__requester__h__
#define __rosidl_typesupport_opensplice_cpp__requester__h__

#include <atomic>
#include <limits>
#include <random>

#include <ccpp_dds_dcps.h>

#include "rosidl_typesupport_opensplice_cpp/message_type_support.h"
#include "rosidl_typesupport_opensplice_cpp/service_type_support.h"

namespace rosidl_typesupport_opensplice_cpp
{

template<typename RequestT, typename ResponseT>
class Requester
{
public:
  Requester(
    DDS::DomainParticipant * participant, const std::string & service_name,
    const std::string & service_type_name)
  : participant_(participant), service_name_(service_name),
    service_type_name_(service_type_name), sequence_number_(0)
  {
    std::random_device rd;
    std::default_random_engine e1(rd());
    // NOTE: use extra parentheses to avoid macro expansion. On Windows,
    // max and min are defined as macros in <windows.h>
    // See http://stackoverflow.com/a/2561377/470581
    std::uniform_int_distribution<uint64_t> uniform_dist(
      (std::numeric_limits<uint64_t>::min)(),
      (std::numeric_limits<uint64_t>::max)());
    writer_guid_.first = uniform_dist(e1);
    writer_guid_.second = uniform_dist(e1);

    // Create request Publisher and DataWriter
    DDS::PublisherQos publisher_qos;
    DDS::ReturnCode_t status;

    // TODO(esteve): check status
    status = participant->get_default_publisher_qos(publisher_qos);

    // TODO(esteve): check that request_publisher_ is valid
    request_publisher_ = participant->create_publisher(
      publisher_qos, NULL, DDS::STATUS_MASK_NONE);

    DDS::TopicQos default_topic_qos;

    // TODO(esteve): check status
    status = participant->get_default_topic_qos(default_topic_qos);

    std::string request_type_name = service_type_name_ + "Request_";
    std::string request_topic_name = service_name_ + "_Request";

    // TODO(esteve): check that request_topic_ is valid
    request_topic_ = participant->create_topic(
      request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
      DDS::STATUS_MASK_NONE);

    DDS::DataWriterQos default_datawriter_qos;

    // TODO(esteve): check status
    status = request_publisher_->get_default_datawriter_qos(default_datawriter_qos);

    // TODO(esteve): check that request_datawriter_ is valid
    request_datawriter_ = request_publisher_->create_datawriter(
      request_topic_, default_datawriter_qos, NULL, DDS::STATUS_MASK_NONE);

    // Create response Subscriber and DataReader
    DDS::SubscriberQos subscriber_qos;
    participant->get_default_subscriber_qos(subscriber_qos);

    // TODO(esteve): check that response_subscriber_ is valid
    response_subscriber_ = participant_->create_subscriber(
      subscriber_qos, NULL, DDS::STATUS_MASK_NONE);

    std::string response_type_name = service_type_name_ + "Response_";
    std::string response_topic_name = service_name_ + "_Response";

    // TODO(esteve): check that response_topic_ is valid
    response_topic_ = participant->create_topic(
      response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
      DDS::STATUS_MASK_NONE);

    DDS::StringSeq args;
    args.length(2);
    args[0] = DDS::string_dup(std::to_string(writer_guid_.first).c_str());
    args[1] = DDS::string_dup(std::to_string(writer_guid_.second).c_str());

    // Let OpenSplice do any needed encoding
    // TODO(esteve): check that content_filtered_response_topic_ is valid
    content_filtered_response_topic_ = participant->create_contentfilteredtopic(
      service_name_.c_str(), response_topic_,
      "client_guid_0_ = %0 AND client_guid_1_ = %1",
      args);

    DDS::DataReaderQos default_datareader_qos;
    response_subscriber_->get_default_datareader_qos(default_datareader_qos);

    // TODO(esteve): check that response_subscriber_ is valid
    response_datareader_ = response_subscriber_->create_datareader(
      content_filtered_response_topic_,
      default_datareader_qos, NULL, DDS::STATUS_MASK_NONE);
  }

  bool take_response(Sample<ResponseT> & response)
  {
    return TemplateDataReader<Sample<ResponseT>>::take_sample(response_datareader_, response);
  }

  void send_request(Sample<RequestT> & request)
  {
    request.sequence_number_ = ++sequence_number_;
    request.client_guid_0_ = writer_guid_.first;
    request.client_guid_1_ = writer_guid_.second;

    TemplateDataWriter<Sample<RequestT>>::write_sample(request_datawriter_, request);
  }

  DDS::DataReader * get_response_datareader()
  {
    return response_datareader_;
  }

private:
  DDS::DomainParticipant * participant_;
  std::string service_name_;
  std::string service_type_name_;
  DDS::DataReader * response_datareader_;
  DDS::DataWriter * request_datawriter_;
  DDS::Topic * response_topic_;
  DDS::ContentFilteredTopic * content_filtered_response_topic_;
  DDS::Topic * request_topic_;
  DDS::Subscriber * response_subscriber_;
  DDS::Publisher * request_publisher_;
  std::atomic<int64_t> sequence_number_;
  std::pair<uint64_t, uint64_t> writer_guid_;
};

}  // namespace rosidl_typesupport_opensplice_cpp

#endif  // __rosidl_typesupport_opensplice_cpp__requester__h__
