#include <iostream>
#include <stdexcept>

#include <ccpp_dds_dcps.h>

#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include "ros_middleware_interface/handles.h"
#include "ros_middleware_opensplice_cpp/MessageTypeSupport.h"

namespace ros_middleware_interface
{

const char * _prismtech_opensplice_identifier = "opensplice_static";

ros_middleware_interface::NodeHandle create_node()
{
    std::cout << "create_node()" << std::endl;

    std::cout << "  create_node() get_instance" << std::endl;
    DDS::DomainParticipantFactory_var dpf_ = DDS::DomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  create_node() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };

    DDS::DomainId_t domain = 0;

    std::cout << "  create_node() create_participant" << std::endl;
    DDS::DomainParticipant * participant = dpf_->create_participant(
        domain, PARTICIPANT_QOS_DEFAULT, NULL,
        DDS::STATUS_MASK_NONE);
    if (!participant) {
        printf("  create_node() could not create participant\n");
        throw std::runtime_error("could not create participant");
    };

    std::cout << "  create_node() pass opaque node handle" << std::endl;

    ros_middleware_interface::NodeHandle node_handle = {
        _prismtech_opensplice_identifier,
        participant
    };
    return node_handle;
}

struct CustomPublisherInfo {
  DDS::DataWriter * topic_writer_;
  ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks_;
};

ros_middleware_interface::PublisherHandle create_publisher(const ros_middleware_interface::NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_publisher()" << std::endl;

    if (node_handle._implementation_identifier != _prismtech_opensplice_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_publisher() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_publisher() extract participant from opaque node handle" << std::endl;
    DDS::DomainParticipant * participant = (DDS::DomainParticipant *)node_handle._data;

    ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = (ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks*)type_support_handle._data;
    std::string type_name = std::string(callbacks->_package_name) + "/" + callbacks->_message_name;


    std::cout << "  create_publisher() invoke register callback" << std::endl;
    callbacks->_register_type(participant, type_name.c_str());


    DDS::PublisherQos publisher_qos;
    DDS::ReturnCode_t status = participant->get_default_publisher_qos(publisher_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_publisher_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default publisher qos failed");
    };

    std::cout << "  create_publisher() create dds publisher" << std::endl;
    DDS::Publisher * dds_publisher = participant->create_publisher(
        publisher_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!dds_publisher) {
        printf("  create_publisher() could not create publisher\n");
        throw std::runtime_error("could not create publisher");
    };


    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_topic_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default topic qos failed");
    };

    std::cout << "  create_publisher() create topic" << std::endl;
    DDS::Topic * topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
    );
    if (!topic) {
        printf("  create_topic() could not create topic\n");
        throw std::runtime_error("could not create topic");
    };


    DDS::DataWriterQos default_datawriter_qos;
    status = dds_publisher->get_default_datawriter_qos(default_datawriter_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_datawriter_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datawriter qos failed");
    };

    std::cout << "  create_publisher() create data writer" << std::endl;
    DDS::DataWriter * topic_writer = dds_publisher->create_datawriter(
        topic, default_datawriter_qos,
        NULL, DDS::STATUS_MASK_NONE);


    std::cout << "  create_publisher() build opaque publisher handle" << std::endl;
    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->topic_writer_ = topic_writer;
    custom_publisher_info->callbacks_ = callbacks;

    ros_middleware_interface::PublisherHandle publisher_handle = {
        _prismtech_opensplice_identifier,
        custom_publisher_info
    };
    return publisher_handle;
}

void publish(const ros_middleware_interface::PublisherHandle& publisher_handle, const void * ros_message)
{
    //std::cout << "publish()" << std::endl;

    if (publisher_handle._implementation_identifier != _prismtech_opensplice_identifier)
    {
        printf("publisher handle not from this implementation\n");
        printf("but from: %s\n", publisher_handle._implementation_identifier);
        throw std::runtime_error("publisher handle not from this implementation");
    }

    //std::cout << "  publish() extract data writer and type code from opaque publisher handle" << std::endl;
    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher_handle._data;
    DDS::DataWriter * topic_writer = custom_publisher_info->topic_writer_;
    const ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = custom_publisher_info->callbacks_;


    //std::cout << "  publish() invoke publish callback" << std::endl;
    callbacks->_publish(topic_writer, ros_message);
}

ros_middleware_interface::SubscriberHandle create_subscriber(const NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
#error TODO
}

void take(const ros_middleware_interface::SubscriberHandle& subscriber_handle, const void * ros_message)
{
#error TODO
}

ros_middleware_interface::GuardConditionHandle create_guard_condition()
{
#error TODO
}

void trigger_guard_condition(const ros_middleware_interface::GuardConditionHandle& guard_condition_handle)
{
#error TODO
}

void wait(ros_middleware_interface::SubscriberHandles& subscriber_handles, ros_middleware_interface::GuardConditionHandles& guard_condition_handles)
{
#error TODO
}

}
