/* Copyright 2015 Open Source Robotics Foundation, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __rosidl_typesupport_opensplice_cpp__responder__h__
#define __rosidl_typesupport_opensplice_cpp__responder__h__

#include <ccpp_dds_dcps.h>
#include <u_entity.h>

#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/service_type_support.h>

namespace rosidl_typesupport_opensplice_cpp
{

template<typename RequestT, typename ResponseT>
class Responder
{
  public:

    Responder(DDS::DomainParticipant_ptr participant, const char * service_name,
      const char * service_type_name) : participant_(participant),
      service_name_(service_name), service_type_name_(service_type_name) {
      // Create request Publisher and DataWriter
      DDS::ReturnCode_t status;
      DDS::TopicQos default_topic_qos;
      status = participant->get_default_topic_qos(default_topic_qos);

      std::string request_type_name = service_type_name_ + "Request_";
      std::string request_topic_name = service_name_ + "_Request";

      request_topic_ = participant->create_topic(
        request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
      );

      // Create response Subscriber and DataReader
      DDS::SubscriberQos subscriber_qos;
      participant->get_default_subscriber_qos(subscriber_qos);
      request_subscriber_ = participant_->create_subscriber(
        subscriber_qos, NULL, DDS::STATUS_MASK_NONE);

      DDS::DataReaderQos default_datareader_qos;
      status = request_subscriber_->get_default_datareader_qos(default_datareader_qos);

      request_datareader_ = request_subscriber_->create_datareader(
        request_topic_,
        default_datareader_qos, NULL, DDS::STATUS_MASK_NONE);

      // Create request Publisher and DataWriter
      DDS::PublisherQos publisher_qos;
      status = participant->get_default_publisher_qos(publisher_qos);

      response_publisher_ = participant->create_publisher(
        publisher_qos, NULL, DDS::STATUS_MASK_NONE);

      std::string response_type_name = service_type_name_ + "Response_";
      std::string response_topic_name = service_name_ + "_Response";

      response_topic_ = participant->create_topic(
        response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
      );

      DDS::DataWriterQos default_datawriter_qos;
      status = response_publisher_->get_default_datawriter_qos(default_datawriter_qos);

      response_datawriter_ = response_publisher_->create_datawriter(
        response_topic_, default_datawriter_qos, NULL, DDS::STATUS_MASK_NONE
      );

    }

    DDS::DataReader * get_request_datareader()
    {
      return request_datareader_;
    }

    bool take_request(Sample<RequestT> &request)
    {
      return TemplateDataReader<Sample <RequestT> >::take_sample(request_datareader_, request);
    }

    void send_response(const rmw_request_id_t &request_header, Sample<ResponseT> &response)
    {
      response.sequence_number_ = request_header.sequence_number;
      response.client_guid_0_ = *(reinterpret_cast<const uint64_t *>(&request_header.writer_guid[0]));
      response.client_guid_1_ = *(reinterpret_cast<const uint64_t *>(
        &request_header.writer_guid[0] + sizeof(response.client_guid_0_)));

      TemplateDataWriter< Sample<ResponseT> >::write_sample(response_datawriter_, response);
    }

  private:

    DDS::DomainParticipant_ptr participant_;
    std::string service_name_;
    std::string service_type_name_;
    DDS::DataReader_ptr request_datareader_;
    DDS::Topic_ptr request_topic_;
    DDS::Subscriber_ptr request_subscriber_;
    DDS::DataWriter_ptr response_datawriter_;
    DDS::Publisher_ptr response_publisher_;
    DDS::Topic_ptr response_topic_;
};

}  // namespace rosidl_typesupport_opensplice_cpp

#endif  // __rosidl_typesupport_opensplice_cpp__responder__h__
