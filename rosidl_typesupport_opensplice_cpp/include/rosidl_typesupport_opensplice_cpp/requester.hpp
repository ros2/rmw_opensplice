#ifndef __rosidl_typesupport_opensplice_cpp__requester__h__
#define __rosidl_typesupport_opensplice_cpp__requester__h__

#include <atomic>
#include <random>
#include <limits>
#include <ccpp_dds_dcps.h>
#include <u_entity.h>

#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>
#include <rosidl_typesupport_opensplice_cpp/service_type_support.h>

namespace rosidl_typesupport_opensplice_cpp
{

template<typename RequestT, typename ResponseT>
class Requester
{
  public:
    Requester(DDS::DomainParticipant_ptr participant, const char * service_name,
              const char * service_type_name) :
      participant_(participant), service_name_(service_name),
      service_type_name_(service_type_name), sequence_number_(0)
    {
      std::random_device rd;
      std::default_random_engine e1(rd());
      std::uniform_int_distribution<uint64_t> uniform_dist(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max());
      writer_guid_.first = uniform_dist(e1);
      writer_guid_.second = uniform_dist(e1);

      // Create request Publisher and DataWriter
      DDS::PublisherQos publisher_qos;
      DDS::ReturnCode_t status;
      status = participant->get_default_publisher_qos(publisher_qos);

      request_publisher_ = participant->create_publisher(
        publisher_qos, NULL, DDS::STATUS_MASK_NONE);

      DDS::TopicQos default_topic_qos;
      status = participant->get_default_topic_qos(default_topic_qos);

      std::string request_type_name = service_type_name_ + "Request_";
      std::string request_topic_name = service_name_ + "_Request";

      request_topic_ = participant->create_topic(
        request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
      );

      DDS::DataWriterQos default_datawriter_qos;
      status = request_publisher_->get_default_datawriter_qos(default_datawriter_qos);

      request_datawriter_ = request_publisher_->create_datawriter(
        request_topic_, default_datawriter_qos, NULL, DDS::STATUS_MASK_NONE
      );

      // Create response Subscriber and DataReader
      DDS::SubscriberQos subscriber_qos;
      participant->get_default_subscriber_qos(subscriber_qos);
      response_subscriber_ = participant_->create_subscriber(
        subscriber_qos, NULL, DDS::STATUS_MASK_NONE);

      std::string response_type_name = service_type_name_ + "Response_";
      std::string response_topic_name = service_name_ + "_Response";

      response_topic_ = participant->create_topic(
        response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
      );

      DDS::StringSeq args;
      args.length(2);
      args[0] = DDS::string_dup(std::to_string(writer_guid_.first).c_str());
      args[1] = DDS::string_dup(std::to_string(writer_guid_.second).c_str());
      // Let OpenSplice do any needed encoding
      content_filtered_response_topic_ = participant->create_contentfilteredtopic(
        service_name_.c_str(), response_topic_,
        "client_guid_0_ = %0 AND client_guid_1_ = %1",
        args);

      DDS::DataReaderQos default_datareader_qos;
      response_subscriber_->get_default_datareader_qos(default_datareader_qos);
      response_datareader_ = response_subscriber_->create_datareader(
        content_filtered_response_topic_,
        default_datareader_qos, NULL, DDS::STATUS_MASK_NONE);
    }

    bool take_response(Sample<ResponseT> &response)
    {
      return TemplateDataReader<Sample <ResponseT> >::take_sample(response_datareader_, response);
    }

    void send_request(Sample<RequestT> &request)
    {
      request.sequence_number_ = ++sequence_number_;
      request.client_guid_0_ = writer_guid_.first;
      request.client_guid_1_ = writer_guid_.second;

      TemplateDataWriter< Sample<RequestT> >::write_sample(request_datawriter_, request);
    }

    DDS::DataReader_ptr get_response_datareader()
    {
      return response_datareader_;
    }

  private:
    DDS::DomainParticipant_ptr participant_;
    std::string service_name_;
    std::string service_type_name_;
    DDS::DataReader_ptr response_datareader_;
    DDS::DataWriter_ptr request_datawriter_;
    DDS::Topic_ptr response_topic_;
    DDS::ContentFilteredTopic_ptr content_filtered_response_topic_;
    DDS::Topic_ptr request_topic_;
    DDS::Subscriber_ptr response_subscriber_;
    DDS::Publisher_ptr request_publisher_;
    std::atomic<int64_t> sequence_number_;
    std::pair<uint64_t, uint64_t> writer_guid_;
};

}  // namespace rosidl_typesupport_opensplice_cpp

#endif  // __rosidl_typesupport_opensplice_cpp__requester__h__
