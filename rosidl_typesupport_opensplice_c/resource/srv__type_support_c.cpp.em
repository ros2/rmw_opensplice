// generated from rosidl_typesupport_opensplice_c/resource/srv__type_support_c.cpp.em
// generated code does not contain a copyright notice

@#########################################e#############################
@# EmPy template for generating <srv>__type_support_c.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.ServiceSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
#include <cstring>
#include <iostream>
#include <sstream>

// This is defined in the rosidl_typesupport_opensplice_c package and
// is in the include/rosidl_typesupport_opensplice_c/impl folder.
#include <rosidl_generator_c/message_type_support.h>
#include <rosidl_typesupport_opensplice_c/visibility_control.h>
#include <rmw/rmw.h>

@{header_file_name = get_header_filename_from_msg_name(spec.srv_name)}@
@{req_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__request')}@
@{res_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__response')}@
#include "@(spec.pkg_name)/srv/@(req_header_file_name).h"
#include "@(spec.pkg_name)/srv/@(res_header_file_name).h"
#include "@(spec.pkg_name)/srv/dds_opensplice/ccpp_@(spec.srv_name)_Request_.h"
#include "@(spec.pkg_name)/srv/dds_opensplice/ccpp_@(spec.srv_name)_Response_.h"
#include "@(spec.pkg_name)/srv/dds_opensplice/ccpp_Sample_@(spec.srv_name)_Request_.h"
#include "@(spec.pkg_name)/srv/dds_opensplice/ccpp_Sample_@(spec.srv_name)_Response_.h"
#include "rosidl_typesupport_opensplice_c/identifier.h"

#include "@(spec.pkg_name)/srv/dds_opensplice/@(get_header_filename_from_msg_name(spec.srv_name))__type_support.cpp"
#include "@(spec.pkg_name)/msg/rosidl_generator_c__visibility_control.h"


@{
# Same as @(spec.pkg_name)::srv::dds_::@(spec.srv_name)
__dds_msg_type_prefix = "{spec.pkg_name}::srv::dds_::{spec.srv_name}".format(spec=spec)
# Same as @(spec.pkg_name)::srv::dds_::Sample_@(spec.srv_name)
__dds_sample_type_prefix = "{spec.pkg_name}::srv::dds_::Sample_{spec.srv_name}".format(spec=spec)
}@

#if defined(__cplusplus)
extern "C"
{
#endif

const char *
register_types__@(spec.srv_name)(
  void * untyped_participant, const char * request_type_name, const char * response_type_name)
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::register_types__@(spec.srv_name)(
    untyped_participant, request_type_name, response_type_name);
}

const char *
create_requester__@(spec.srv_name)(
  void * untyped_participant, const char * service_name,
  void **untyped_requester, void ** untyped_reader,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void * (* allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::create_requester__@(spec.srv_name)(
    untyped_participant, service_name,
    untyped_requester, untyped_reader,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    allocator);
}

const char *
create_responder__@(spec.srv_name)(
  void * untyped_participant, const char * service_name,
  void **untyped_responder, void **untyped_reader,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void * (* allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::create_responder__@(spec.srv_name)(
    untyped_participant, service_name,
    untyped_responder, untyped_reader,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    allocator);
}

const char *
send_request__@(spec.srv_name)(
  void * untyped_requester, const void * untyped_ros_request, int64_t * sequence_number)
{
  using SampleT = rosidl_typesupport_opensplice_cpp::Sample<@(__dds_msg_type_prefix)_Request_>;

  SampleT request;
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_TYPE_SUPPORT(@(spec.pkg_name), srv, @(spec.srv_name)_Request);
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  callbacks->convert_ros_to_dds(untyped_ros_request, static_cast<void *>(&request.data()));

  using RequesterT = rosidl_typesupport_opensplice_cpp::Requester<
    @(__dds_msg_type_prefix)_Request_,
    @(__dds_msg_type_prefix)_Response_
  >;

  auto requester = reinterpret_cast<RequesterT *>(untyped_requester);

  const char * error_string = requester->send_request(request);
  if (error_string) {
    return error_string;
  }
  *sequence_number = request.sequence_number_;

  return nullptr;
}

const char *
take_request__@(spec.srv_name)(
  void * untyped_responder, rmw_request_id_t * request_header, void * untyped_ros_request,
  bool * taken)
{
  using ResponderT = rosidl_typesupport_opensplice_cpp::Responder<
    @(__dds_msg_type_prefix)_Request_,
    @(__dds_msg_type_prefix)_Response_
  >;

  auto responder = reinterpret_cast<ResponderT *>(untyped_responder);

  rosidl_typesupport_opensplice_cpp::Sample<@(__dds_msg_type_prefix)_Request_> request;
  const char * error_string = responder->take_request(request, taken);
  if (error_string) {
    return error_string;
  }

  if (*taken) {
    const rosidl_message_type_support_t * ts =
      ROSIDL_GET_TYPE_SUPPORT(@(spec.pkg_name), srv, @(spec.srv_name)_Request);
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(ts->data);
    callbacks->convert_dds_to_ros(static_cast<void *>(&request.data()), untyped_ros_request);

    request_header->sequence_number = request.sequence_number_;
    std::memcpy(
      &request_header->writer_guid[0], &request.client_guid_0_, sizeof(request.client_guid_0_));
    std::memcpy(
      &request_header->writer_guid[0] + sizeof(request.client_guid_0_),
      &request.client_guid_1_, sizeof(request.client_guid_1_));

    *taken = true;
    return nullptr;
  }
  *taken = false;
  return nullptr;
}

const char *
send_response__@(spec.srv_name)(
  void * untyped_responder, const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  rosidl_typesupport_opensplice_cpp::Sample<@(__dds_msg_type_prefix)_Response_> response;
  const rosidl_message_type_support_t * ts =
    ROSIDL_GET_TYPE_SUPPORT(@(spec.pkg_name), srv, @(spec.srv_name)_Response);
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  callbacks->convert_ros_to_dds(untyped_ros_response, static_cast<void *>(&response.data()));

  using ResponderT = rosidl_typesupport_opensplice_cpp::Responder<
    @(__dds_msg_type_prefix)_Request_,
    @(__dds_msg_type_prefix)_Response_
  >;
  auto responder = reinterpret_cast<ResponderT *>(untyped_responder);

  const char * error_string = responder->send_response(*request_header, response);
  if (error_string) {
    return error_string;
  }
  return nullptr;
}

const char *
take_response__@(spec.srv_name)(
  void * untyped_requester, rmw_request_id_t * request_header, void * untyped_ros_response,
  bool * taken)
{
  using RequesterT = rosidl_typesupport_opensplice_cpp::Requester<
    @(__dds_msg_type_prefix)_Request_,
    @(__dds_msg_type_prefix)_Response_
  >;
  auto requester = reinterpret_cast<RequesterT *>(untyped_requester);

  rosidl_typesupport_opensplice_cpp::Sample<@(__dds_msg_type_prefix)_Response_> response;
  const char * error_string = requester->take_response(response, taken);
  if (error_string) {
    return error_string;
  }
  if (*taken) {
    request_header->sequence_number = response.sequence_number_;

    const rosidl_message_type_support_t * ts =
      ROSIDL_GET_TYPE_SUPPORT(@(spec.pkg_name), srv, @(spec.srv_name)_Response);
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(ts->data);
    callbacks->convert_dds_to_ros(
      static_cast<void *>(&response.data()), untyped_ros_response);
    return nullptr;
  }

  return nullptr;
}

const char *
destroy_requester__@(spec.srv_name)(void * untyped_requester, void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::destroy_requester__@(spec.srv_name)(
    untyped_requester, deallocator);
}

const char *
destroy_responder__@(spec.srv_name)(void * untyped_responder, void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::destroy_responder__@(spec.srv_name)(
    untyped_responder, deallocator);
}

const char *
server_is_available__@(spec.srv_name)(
  void * requester, const rmw_node_t * node, bool * is_available,
  rmw_ret_t (*count_publishers)(const rmw_node_t *, const char *, size_t *),
  rmw_ret_t (*count_subscribers)(const rmw_node_t *, const char *, size_t *))
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::server_is_available__@(spec.srv_name)(
    requester, node, is_available, count_publishers, count_subscribers);
}

static service_type_support_callbacks_t __callbacks = {
  "@(spec.pkg_name)",
  "@(spec.srv_name)",
  &create_requester__@(spec.srv_name),
  &destroy_requester__@(spec.srv_name),
  &create_responder__@(spec.srv_name),
  &destroy_responder__@(spec.srv_name),
  &send_request__@(spec.srv_name),
  &take_request__@(spec.srv_name),
  &send_response__@(spec.srv_name),
  &take_response__@(spec.srv_name),
  &server_is_available__@(spec.srv_name),
};

static rosidl_service_type_support_t __type_support = {
  // Cannot be set since it is not a constexpr, it is set in the get type support function below.
  0,  // typesupport_identifier
  &__callbacks,  // data
};

ROSIDL_GENERATOR_C_EXPORT_@(spec.pkg_name)
const rosidl_service_type_support_t *
ROSIDL_GET_TYPE_SUPPORT_FUNCTION(@(spec.pkg_name), srv, @(spec.srv_name))()
{
  if (!__type_support.typesupport_identifier) {
    __type_support.typesupport_identifier = rosidl_typesupport_opensplice_c__identifier;
  }
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif
