// generated from rosidl_typesupport_opensplice_cpp/resource/msg__type_support.cpp.em

@#######################################################################
@# EmPy template for generating <msg>__type_support.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.MessageSpecification)
@#    Parsed specification of the .msg file
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
#include <limits>
#include <stdexcept>

#include <u_instanceHandle.h>

#include <rosidl_generator_c/message_type_support_struct.h>
// this is defined in the rosidl_typesupport_opensplice_cpp package and
// is in the include/rosidl_typesupport_opensplice_cpp/impl folder
#include <rosidl_generator_cpp/message_type_support.hpp>
#include <rosidl_typesupport_opensplice_cpp/visibility_control.h>

@{header_file_name = get_header_filename_from_msg_name(spec.base_type.type)}@
#include "@(spec.base_type.pkg_name)/@(subfolder)/@(header_file_name)__struct.hpp"
#include "@(spec.base_type.pkg_name)/@(subfolder)/dds_opensplice/@(header_file_name)__type_support.hpp"
#include "@(spec.base_type.pkg_name)/@(subfolder)/dds_opensplice/ccpp_@(spec.base_type.type)_.h"
#include "rosidl_typesupport_opensplice_cpp/identifier.hpp"
#include <rosidl_typesupport_opensplice_cpp/message_type_support.h>

// forward declaration of message dependencies and their conversion functions
@[for field in spec.fields]@
@[  if not field.type.is_primitive_type()]@
@[    if field.type.pkg_name == 'builtin_interfaces']@
#include "rosidl_typesupport_opensplice_cpp/duration__type_support.hpp"
#include "rosidl_typesupport_opensplice_cpp/time__type_support.hpp"
@[    else]@
namespace @(field.type.pkg_name)
{
namespace msg
{
namespace dds_
{
struct @(field.type.type)_;
}  // namespace dds_
namespace typesupport_opensplice_cpp
{
void convert_ros_message_to_dds(
  const @(field.type.pkg_name)::msg::@(field.type.type) &,
  @(field.type.pkg_name)::msg::dds_::@(field.type.type)_ &);
void convert_dds_message_to_ros(
  const @(field.type.pkg_name)::msg::dds_::@(field.type.type)_ &,
  @(field.type.pkg_name)::msg::@(field.type.type) &);
}  // namespace typesupport_opensplice_cpp
}  // namespace msg
}  // namespace @(field.type.pkg_name)
@[    end if]@

@[  end if]@
@[end for]@

namespace @(spec.base_type.pkg_name)
{

namespace @(subfolder)
{

namespace typesupport_opensplice_cpp
{

@{
__dds_msg_type_prefix = "{}::{}::dds_::{}_".format(
  spec.base_type.pkg_name, subfolder, spec.base_type.type)
}@
using __dds_msg_type = @(__dds_msg_type_prefix);
using __ros_msg_type = @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type);

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT_@(spec.base_type.pkg_name)
const char *
register_type__@(spec.base_type.type)(
  void * untyped_participant,
  const char * type_name)
{
  if (!untyped_participant) {
    return "untyped participant handle is null";
  }
  if (!type_name) {
    return "type name handle is null";
  }
  DDS::DomainParticipant * participant =
    static_cast<DDS::DomainParticipant *>(untyped_participant);

  @(__dds_msg_type_prefix)TypeSupport type_support;
  DDS::ReturnCode_t status = type_support.register_type(participant, type_name);
  switch (status) {
    case DDS::RETCODE_ERROR:
      return "@(__dds_msg_type_prefix)TypeSupport.register_type: "
             "an internal error has occurred";
    case DDS::RETCODE_BAD_PARAMETER:
      return "@(__dds_msg_type_prefix)TypeSupport.register_type: "
             "bad domain participant or type name parameter";
    case DDS::RETCODE_OUT_OF_RESOURCES:
      return "@(__dds_msg_type_prefix)TypeSupport.register_type: "
             "out of resources";
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      return "@(__dds_msg_type_prefix)TypeSupport.register_type: "
             "already registered with a different TypeSupport class";
    case DDS::RETCODE_OK:
      return nullptr;
    default:
      return "@(__dds_msg_type_prefix)TypeSupport.register_type: unknown return code";
  }
}

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT_@(spec.base_type.pkg_name)
void
convert_ros_message_to_dds(const __ros_msg_type & ros_message, __dds_msg_type & dds_message)
{
@[if not spec.fields]@
  (void)ros_message;
  (void)dds_message;
@[end if]@
@[for field in spec.fields]@
  // field.name @(field.name)
@[  if field.type.is_array]@
  {
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = ros_message.@(field.name).size();
    if (size > (std::numeric_limits<DDS::Long>::max)()) {
      throw std::runtime_error("array size exceeds maximum DDS sequence size");
    }
    DDS::Long length = static_cast<DDS::Long>(size);
    dds_message.@(field.name)_.length(length);
@[    end if]@
    for (DDS::ULong i = 0; i < size; i++) {
@[    if field.type.type == 'string']@
      dds_message.@(field.name)_[i] = ros_message.@(field.name)[i].c_str();
@[    elif field.type.type == 'bool']@
      dds_message.@(field.name)_[i] = 1 ? ros_message.@(field.name)[i] : 0;
@[    elif field.type.is_primitive_type()]@
      dds_message.@(field.name)_[i] = ros_message.@(field.name)[i];
@[    else]@
      @(field.type.pkg_name)::msg::typesupport_opensplice_cpp::convert_ros_message_to_dds(
        ros_message.@(field.name)[i], dds_message.@(field.name)_[i]);
@[    end if]@
    }
  }
@[  elif field.type.type == 'string']@
  dds_message.@(field.name)_ = ros_message.@(field.name).c_str();
@[  elif field.type.is_primitive_type()]@
  dds_message.@(field.name)_ = ros_message.@(field.name);
@[  else]@
  @(field.type.pkg_name)::msg::typesupport_opensplice_cpp::convert_ros_message_to_dds(
    ros_message.@(field.name), dds_message.@(field.name)_);
@[  end if]@

@[end for]@
}

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT_@(spec.base_type.pkg_name)
const char *
publish__@(spec.base_type.type)(
  void * untyped_topic_writer,
  const void * untyped_ros_message)
{
  DDS::DataWriter * topic_writer = static_cast<DDS::DataWriter *>(untyped_topic_writer);

  const __ros_msg_type & ros_message = *(const __ros_msg_type *)untyped_ros_message;
  __dds_msg_type dds_message;
  convert_ros_message_to_dds(ros_message, dds_message);

  @(__dds_msg_type_prefix)DataWriter * data_writer =
    @(__dds_msg_type_prefix)DataWriter::_narrow(topic_writer);
  DDS::ReturnCode_t status = data_writer->write(dds_message, DDS::HANDLE_NIL);
  switch (status) {
    case DDS::RETCODE_ERROR:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "an internal error has occurred";
    case DDS::RETCODE_BAD_PARAMETER:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "bad handle or instance_data parameter";
    case DDS::RETCODE_ALREADY_DELETED:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "this @(__dds_msg_type_prefix)DataWriter has already been deleted";
    case DDS::RETCODE_OUT_OF_RESOURCES:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "out of resources";
    case DDS::RETCODE_NOT_ENABLED:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "this @(__dds_msg_type_prefix)DataWriter is not enabled";
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "the handle has not been registered with this @(__dds_msg_type_prefix)DataWriter";
    case DDS::RETCODE_TIMEOUT:
      return "@(__dds_msg_type_prefix)DataWriter.write: "
             "writing resulted in blocking and then exceeded the timeout set by the "
             "max_blocking_time of the ReliabilityQosPolicy";
    case DDS::RETCODE_OK:
      return nullptr;
    default:
      return "@(__dds_msg_type_prefix)DataWriter.write: unknown return code";
  }
}

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT_@(spec.base_type.pkg_name)
void
convert_dds_message_to_ros(const __dds_msg_type & dds_message, __ros_msg_type & ros_message)
{
@[if not spec.fields]@
  (void)ros_message;
  (void)dds_message;
@[end if]@
@[for field in spec.fields]@
  // field.name @(field.name)
@[  if field.type.is_array]@
  {
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = dds_message.@(field.name)_.length();
    ros_message.@(field.name).resize(size);
@[    end if]@
    for (DDS::ULong i = 0; i < size; i++) {
@[    if field.type.type == 'bool']@
      ros_message.@(field.name)[i] = (dds_message.@(field.name)_[i] != 0);
@[    elif field.type.is_primitive_type()]@
      ros_message.@(field.name)[i] =
        dds_message.@(field.name)_[i]@(' == TRUE' if field.type.type == 'bool' else '');
@[    else]@
      @(field.type.pkg_name)::msg::typesupport_opensplice_cpp::convert_dds_message_to_ros(
        dds_message.@(field.name)_[i], ros_message.@(field.name)[i]);
@[    end if]@
    }
  }
@[  elif field.type.is_primitive_type()]@
  ros_message.@(field.name) =
    dds_message.@(field.name)_@(' == TRUE' if field.type.type == 'bool' else '');
@[  else]@
  @(field.type.pkg_name)::msg::typesupport_opensplice_cpp::convert_dds_message_to_ros(
    dds_message.@(field.name)_, ros_message.@(field.name));
@[  end if]@

@[end for]@
}

ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT_@(spec.base_type.pkg_name)
const char *
take__@(spec.base_type.type)(
  void * untyped_topic_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle)
{
  if (untyped_ros_message == 0) {
    return "invalid ros message pointer";
  };

  DDS::DataReader * topic_reader = static_cast<DDS::DataReader*>(untyped_topic_reader);

  @(__dds_msg_type_prefix)DataReader * data_reader =
    @(__dds_msg_type_prefix)DataReader::_narrow(topic_reader);

  @(__dds_msg_type_prefix)Seq dds_messages;
  DDS::SampleInfoSeq sample_infos;
  DDS::ReturnCode_t status = data_reader->take(
    dds_messages,
    sample_infos,
    1,
    DDS::ANY_SAMPLE_STATE,
    DDS::ANY_VIEW_STATE,
    DDS::ANY_INSTANCE_STATE);

  const char * errs = nullptr;
  bool ignore_sample = false;

  switch (status) {
    case DDS::RETCODE_ERROR:
      errs = "@(__dds_msg_type_prefix)DataReader.take: "
             "an internal error has occurred";
      goto finally;
    case DDS::RETCODE_ALREADY_DELETED:
      errs = "@(__dds_msg_type_prefix)DataReader.take: "
             "this @(__dds_msg_type_prefix)DataReader has already been deleted";
      goto finally;
    case DDS::RETCODE_OUT_OF_RESOURCES:
      errs = "@(__dds_msg_type_prefix)DataReader.take: "
             "out of resources";
      goto finally;
    case DDS::RETCODE_NOT_ENABLED:
      errs = "@(__dds_msg_type_prefix)DataReader.take: "
             "this @(__dds_msg_type_prefix)DataReader is not enabled";
      goto finally;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      errs = "@(__dds_msg_type_prefix)DataReader.take: "
             "a precondition is not met, one of: "
             "max_samples > maximum and max_samples != LENGTH_UNLIMITED, or "
             "the two sequences do not have matching parameters (length, maximum, release), or "
             "maximum > 0 and release is false.";
      goto finally;
    case DDS::RETCODE_NO_DATA:
      *taken = false;
      errs = nullptr;
      goto finally;
    case DDS::RETCODE_OK:
      break;
    default:
      errs = "@(__dds_msg_type_prefix)DataReader.take: unknown return code";
      goto finally;
  }

  {
    DDS::SampleInfo & sample_info = sample_infos[0];
    if (!sample_info.valid_data) {
      // skip sample without data
      ignore_sample = true;
    } else {
      DDS::InstanceHandle_t sender_handle = sample_info.publication_handle;
      auto sender_gid = u_instanceHandleToGID(sender_handle);
      if (ignore_local_publications) {
        // compare the system id from the sender and this receiver
        // if they are equal the sample has been sent from this process and should be ignored
        DDS::InstanceHandle_t receiver_handle = topic_reader->get_instance_handle();
        auto receiver_gid = u_instanceHandleToGID(receiver_handle);
        ignore_sample = sender_gid.systemId == receiver_gid.systemId;
      }
      // This is nullptr when being used with plain rmw_take, so check first.
      if (sending_publication_handle) {
        *static_cast<DDS::InstanceHandle_t *>(sending_publication_handle) = sender_handle;
      }
    }
  }

  if (!ignore_sample) {
    __ros_msg_type & ros_message = *(__ros_msg_type *)untyped_ros_message;
    convert_dds_message_to_ros(dds_messages[0], ros_message);
    *taken = true;
  } else {
    *taken = false;
  }

finally:
  // Ensure the loan is returned.
  status = data_reader->return_loan(dds_messages, sample_infos);
  switch (status) {
    case DDS::RETCODE_ERROR:
      return "@(__dds_msg_type_prefix)DataReader.return_loan: "
             "an internal error has occurred";
    case DDS::RETCODE_ALREADY_DELETED:
      return "@(__dds_msg_type_prefix)DataReader.return_loan: "
             "this @(__dds_msg_type_prefix)DataReader has already been deleted";
    case DDS::RETCODE_OUT_OF_RESOURCES:
      return "@(__dds_msg_type_prefix)DataReader.return_loan: "
             "out of resources";
    case DDS::RETCODE_NOT_ENABLED:
      return "@(__dds_msg_type_prefix)DataReader.return_loan: "
             "this @(__dds_msg_type_prefix)DataReader is not enabled";
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      return "@(__dds_msg_type_prefix)DataReader.return_loan: "
             "a precondition is not met, one of: "
             "the data_values and info_seq do not belong to a single related pair, or "
             "the data_values and info_seq were not obtained from this "
             "@(__dds_msg_type_prefix)DataReader";
    case DDS::RETCODE_OK:
      return nullptr;
    default:
      return "@(__dds_msg_type_prefix)DataReader.return_loan failed with "
             "unknown return code";
  }

  return errs;
}

static message_type_support_callbacks_t callbacks = {
  "@(spec.base_type.pkg_name)",
  "@(spec.base_type.type)",
  &register_type__@(spec.base_type.type),
  &publish__@(spec.base_type.type),
  &take__@(spec.base_type.type),
  nullptr,  // convert ros to dds (handled differently for C++)
  nullptr,  // convert dds to ros (handled differently for C++)
};

static rosidl_message_type_support_t handle = {
  rosidl_typesupport_opensplice_cpp::typesupport_opensplice_identifier,
  &callbacks
};

}  // namespace typesupport_opensplice_cpp

}  // namespace @(subfolder)

}  // namespace @(spec.base_type.pkg_name)

namespace rosidl_typesupport_opensplice_cpp
{

template<>
ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_EXPORT
const rosidl_message_type_support_t *
get_message_type_support_handle_opensplice<
  @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type)
>()
{
  return &@(spec.base_type.pkg_name)::@(subfolder)::typesupport_opensplice_cpp::handle;
}

}  // namespace rosidl_typesupport_opensplice_cpp
