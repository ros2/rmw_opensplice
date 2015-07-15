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

#include "rosidl_typesupport_opensplice_cpp/impl/error_checking.hpp"

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
  {}

  const char * init() noexcept
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

    DDS::StringSeq args;
    args.length(2);
    args[0] = DDS::string_dup(std::to_string(writer_guid_.first).c_str());
    args[1] = DDS::string_dup(std::to_string(writer_guid_.second).c_str());

    DDS::ReturnCode_t status;
    DDS::TopicQos default_topic_qos;
    DDS::PublisherQos publisher_qos;
    DDS::DataWriterQos default_datawriter_qos;
    DDS::SubscriberQos subscriber_qos;
    DDS::DataReaderQos default_datareader_qos;
    std::string request_type_name = service_type_name_ + "_Request_";
    std::string request_topic_name = service_name_ + "_Request";
    std::string response_type_name = service_type_name_ + "_Response_";
    std::string response_topic_name = service_name_ + "_Response";
    const char * estr = nullptr;
    request_topic_ = nullptr;
    request_publisher_ = nullptr;
    request_datawriter_ = nullptr;
    response_topic_ = nullptr;
    response_subscriber_ = nullptr;
    response_datareader_ = nullptr;
    content_filtered_response_topic_ = nullptr;

    // Create request Publisher and DataWriter
    status = participant_->get_default_publisher_qos(publisher_qos);
    if (nullptr != (estr = impl::check_get_default_publisher_qos(status))) {
      goto fail;
    }

    request_publisher_ =
      participant_->create_publisher(publisher_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!request_publisher_) {
      estr = "DomainParticipant::create_publisher: failed for request";
      goto fail;
    }

    status = participant_->get_default_topic_qos(default_topic_qos);
    if (nullptr != (estr = impl::check_get_default_topic_qos(status))) {
      goto fail;
    }

    request_topic_ = participant_->create_topic(
      request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
      DDS::STATUS_MASK_NONE);
    if (!request_topic_) {
      estr = "DomainParticipant::create_topic: failed for request";
      goto fail;
    }

    status = request_publisher_->get_default_datawriter_qos(default_datawriter_qos);
    if (nullptr != (estr = impl::check_get_default_datawriter_qos(status))) {
      goto fail;
    }

    request_datawriter_ = request_publisher_->create_datawriter(
      request_topic_, default_datawriter_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!request_datawriter_) {
      estr = "Publisher::create_datawriter: failed for request";
      goto fail;
    }

    // Create response Subscriber and DataReader
    status = participant_->get_default_subscriber_qos(subscriber_qos);
    if (nullptr != (estr = impl::check_get_default_datareader_qos(status))) {
      goto fail;
    }

    response_subscriber_ = participant_->create_subscriber(
      subscriber_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!response_subscriber_) {
      estr = "DomainParticipant::create_subscriber: failed for response";
      goto fail;
    }

    response_topic_ = participant_->create_topic(
      response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
      DDS::STATUS_MASK_NONE);
    if (!response_topic_) {
      estr = "DomainParticipant::create_topic: failed for response";
      goto fail;
    }

    // Let OpenSplice do any needed encoding
    content_filtered_response_topic_ = participant_->create_contentfilteredtopic(
      service_name_.c_str(), response_topic_,
      "client_guid_0_ = %0 AND client_guid_1_ = %1",
      args);
    if (!content_filtered_response_topic_) {
      estr = "DomainParticipant::create_contentfilteredtopic: failed";
      goto fail;
    }

    status = response_subscriber_->get_default_datareader_qos(default_datareader_qos);
    if (nullptr != (estr = impl::check_get_default_datareader_qos(status))) {
      goto fail;
    }

    response_datareader_ = response_subscriber_->create_datareader(
      content_filtered_response_topic_,
      default_datareader_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!response_datareader_) {
      estr = "Subscriber::create_datawriter: failed for response";
      goto fail;
    }
    return nullptr;
fail:
    if (content_filtered_response_topic_) {
      status = participant_->delete_contentfilteredtopic(content_filtered_response_topic_);
      if (nullptr != impl::check_delete_contentfilteredtopic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_contentfilteredtopic(status));
      }
    }
    if (response_datareader_) {
      // Assumption: subscriber is not null at this point.
      status = response_subscriber_->delete_datareader(response_datareader_);
      if (nullptr != impl::check_delete_datareader(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datareader(status));
      }
    }
    if (response_subscriber_) {
      status = participant_->delete_subscriber(response_subscriber_);
      if (nullptr != impl::check_delete_subscriber(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_subscriber(status));
      }
    }
    if (response_topic_) {
      status = participant_->delete_topic(response_topic_);
      if (nullptr != impl::check_delete_topic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_topic(status));
      }
    }
    if (request_datawriter_) {
      // Assumption: publisher is not null at this point.
      status = request_publisher_->delete_datawriter(request_datawriter_);
      if (nullptr != impl::check_delete_datawriter(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datawriter(status));
      }
    }
    if (request_publisher_) {
      status = participant_->delete_publisher(request_publisher_);
      if (nullptr != impl::check_delete_publisher(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_publisher(status));
      }
    }
    if (request_topic_) {
      status = participant_->delete_topic(request_topic_);
      if (nullptr != impl::check_delete_topic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_topic(status));
      }
    }
    return estr;
  }

  const char * take_response(Sample<ResponseT> & response, bool * taken) noexcept
  {
    return TemplateDataReader<Sample<ResponseT>>::take_sample(
      response_datareader_, response, taken);
  }

  const char * send_request(Sample<RequestT> & request) noexcept
  {
    request.sequence_number_ = ++sequence_number_;
    request.client_guid_0_ = writer_guid_.first;
    request.client_guid_1_ = writer_guid_.second;

    return TemplateDataWriter<Sample<RequestT>>::write_sample(request_datawriter_, request);
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
