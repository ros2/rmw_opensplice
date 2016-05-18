// generated from rosidl_typesupport_opensplice_c/resource/msg__type_support.h.em

@##########################################################################
@# EmPy template for generating <msg>__type_support.h files for OpenSplice
@#
@# Context:
@#  - spec (rosidl_parser.MessageSpecification)
@#    Parsed specification of the .msg file
@#  - pkg (string)
@#    name of the containing package; equivalent to spec.base_type.pkg_name
@#  - msg (string)
@#    name of the message; equivalent to spec.msg_name
@#  - type (string)
@#    full type of the message; equivalent to spec.base_type.type
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_header_filename_from_msg_name (function)
@##########################################################################
@
#ifndef __@(pkg)__@(subfolder)__dds_opensplice_c__@(get_header_filename_from_msg_name(type))__type_support__hpp__
#define __@(pkg)__@(subfolder)__dds_opensplice_c__@(get_header_filename_from_msg_name(type))__type_support__hpp__

#if defined(__cplusplus)
extern "C"
{
#endif


const char *
@(pkg)__@(subfolder)__@(msg)__register_type(
  void * untyped_participant, const char * type_name);

const char *
@(pkg)__@(subfolder)__@(msg)__convert_ros_to_dds(
  const void * untyped_ros_message, void * untyped_dds_message);

const char *
@(pkg)__@(subfolder)__@(msg)__publish(void * dds_data_writer, const void * ros_message);

const char *
@(pkg)__@(subfolder)__@(msg)__convert_dds_to_ros(
  const void * untyped_dds_message, void * untyped_ros_message);

const char *
@(pkg)__@(subfolder)__@(msg)__take(
  void * dds_data_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle);


#if defined(__cplusplus)
}
#endif

#endif  // __@(pkg)__@(subfolder)__dds_opensplice_c__@(get_header_filename_from_msg_name(type))__type_support__hpp__
