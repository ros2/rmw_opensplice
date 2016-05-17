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
#include "rosidl_generator_c/message_type_support.h"
#include "rosidl_typesupport_opensplice_c/visibility_control.h"
#include "rmw/rmw.h"

@{header_file_name = get_header_filename_from_msg_name(spec.srv_name)}@
@{req_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__request')}@
@{res_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__response')}@
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
  void ** untyped_requester, void ** untyped_reader,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void * (*allocator)(size_t))
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
  void ** untyped_responder, void ** untyped_reader,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void * (*allocator)(size_t))
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
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::send_request__@(spec.srv_name)(
    untyped_requester, untyped_ros_request, sequence_number);
}

const char *
take_request__@(spec.srv_name)(
  void * untyped_responder, rmw_request_id_t * request_header, void * untyped_ros_request,
  bool * taken)
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::take_request__@(spec.srv_name)(
    untyped_responder, request_header, untyped_ros_request, taken);
}

const char *
send_response__@(spec.srv_name)(
  void * untyped_responder, const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::send_response__@(spec.srv_name)(
    untyped_responder, request_header, untyped_ros_response);
}

const char *
take_response__@(spec.srv_name)(
  void * untyped_requester, rmw_request_id_t * request_header, void * untyped_ros_response,
  bool * taken)
{
  return @(spec.pkg_name)::srv::typesupport_opensplice_cpp::take_response__@(spec.srv_name)(
    untyped_requester, request_header, untyped_ros_response,
    taken);
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
};

static rosidl_service_type_support_t __type_support = {
  // Cannot be set since it is not a constexpr, it is set in the get type support function below.
  0,  // typesupport_identifier
  &__callbacks,  // data
};

ROSIDL_GENERATOR_C_EXPORT_@(spec.pkg_name)
const rosidl_service_type_support_t *
ROSIDL_GET_TYPE_SUPPORT_FUNCTION(@(spec.pkg_name), srv, @(spec.srv_name))() {
  if (!__type_support.typesupport_identifier) {
    __type_support.typesupport_identifier = rosidl_typesupport_opensplice_c__identifier;
  }
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif
