// generated from rosidl_typesupport_opensplice_c/resource/msg__type_support_c.cpp.em
// generated code does not contain a copyright notice

@##########################################################################
@# EmPy template for generating <msg>__type_support_c.cpp files for OpenSplice
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
#include "@(spec.base_type.pkg_name)/@(subfolder)/@(get_header_filename_from_msg_name(spec.base_type.type))__rosidl_typesupport_opensplice_c.h"

#include <cassert>
#include <limits>

#include <u_instanceHandle.h>

// Provides the rosidl_typesupport_opensplice_c__identifier symbol declaration.
#include "rosidl_typesupport_opensplice_c/identifier.h"
#include "@(spec.base_type.pkg_name)/msg/rosidl_generator_c__visibility_control.h"
// Provides the definition of the message_type_support_callbacks_t struct.
#include "rosidl_typesupport_opensplice_cpp/message_type_support.h"
#include "rosidl_typesupport_opensplice_cpp/u__instanceHandle.h"

#include "@(pkg)/msg/rosidl_typesupport_opensplice_c__visibility_control.h"
@{header_file_name = get_header_filename_from_msg_name(type)}@
#include "@(pkg)/@(subfolder)/@(header_file_name)__struct.h"
#include "@(pkg)/@(subfolder)/@(header_file_name)__functions.h"
#include "@(pkg)/@(subfolder)/dds_opensplice/ccpp_@(type)_.h"

// includes and forward declarations of message dependencies and their conversion functions
@# // Include the duration header if needed.
@[for field in spec.fields]@
@[  if not field.type.is_primitive_type() and field.type.type == 'builtin_interfaces__duration']@
#include "rosidl_typesupport_opensplice_c/duration__type_support.h"
@[    break]@
@[  end if]@
@[end for]@
@# // Include the time header if needed.
@[for field in spec.fields]@
@[  if not field.type.is_primitive_type() and field.type.type == 'builtin_interfaces__time']@
#include "rosidl_typesupport_opensplice_c/time__type_support.h"
@[    break]@
@[  end if]@
@[end for]@
@# // Include the message header for each non-primitive field.
#if defined(__cplusplus)
extern "C"
{
#endif

// include message dependencies
@{
includes = {}
for field in spec.fields:
    keys = set([])
    if field.type.is_primitive_type():
        if field.type.is_array:
            keys.add('rosidl_generator_c/primitives_array.h')
            keys.add('rosidl_generator_c/primitives_array_functions.h')
        if field.type.type == 'string':
            keys.add('rosidl_generator_c/string.h')
            keys.add('rosidl_generator_c/string_functions.h')
    else:
        header_file_name = get_header_filename_from_msg_name(field.type.type)
        keys.add('%s/msg/%s__functions.h' % (field.type.pkg_name, header_file_name))
    for key in keys:
        if key not in includes:
            includes[key] = set([])
        includes[key].add(field.name)
}@
@[for key in sorted(includes.keys())]@
#include "@(key)"  // @(', '.join(includes[key]))
@[end for]@

// forward declare type support functions
@{
forward_declares = {}
for field in spec.fields:
    if not field.type.is_primitive_type():
        key = (field.type.pkg_name, field.type.type)
        if key not in includes:
            forward_declares[key] = set([])
        forward_declares[key].add(field.name)
}@
@[for key in sorted(forward_declares.keys())]@
@[  if key[0] != pkg]@
ROSIDL_TYPESUPPORT_OPENSPLICE_C_IMPORT_@(pkg)
@[  end if]@
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_opensplice_c, @(key[0]), msg, @(key[1]))();
@[end for]@

@# // Make callback functions specific to this message type.
@{
__dds_msg_type_prefix = "{0}::{1}::dds_::{2}_".format(
  spec.base_type.pkg_name, subfolder, spec.base_type.type)
}@
using __dds_msg_type = @(__dds_msg_type_prefix);
using __ros_msg_type = @(pkg)__@(subfolder)__@(type);

static const char *
register_type(void * untyped_participant, const char * type_name)
{
  if (!untyped_participant) {
    return "untyped participant handle is null";
  }
  if (!type_name) {
    return "type name handle is null";
  }
  using DDS::DomainParticipant;
  DomainParticipant * participant = static_cast<DomainParticipant *>(untyped_participant);

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

static const char *
convert_ros_to_dds(const void * untyped_ros_message, void * untyped_dds_message)
{
  if (!untyped_ros_message) {
    return "ros message handle is null";
  }
  if (!untyped_dds_message) {
    return "dds message handle is null";
  }
  const __ros_msg_type * ros_message = static_cast<const __ros_msg_type *>(untyped_ros_message);
  __dds_msg_type * dds_message = static_cast<__dds_msg_type *>(untyped_dds_message);
@[if not spec.fields]@
  (void)dds_message;
  (void)ros_message;
  return 0;  // No fields is a no-op.
@[end if]@
@[for field in spec.fields]@
  // Field name: @(field.name)
  {
@[  if not field.type.is_primitive_type()]@
    const message_type_support_callbacks_t * @(field.type.pkg_name)__msg__@(field.type.type)__callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_opensplice_c, @(field.type.pkg_name), msg, @(field.type.type)
      )()->data);
@[  end if]@
@[  if field.type.is_array]@
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = ros_message->@(field.name).size;
    if (size > (std::numeric_limits<DDS::Long>::max)()) {
      return "array size exceeds maximum DDS sequence size";
    }
    DDS::Long length = static_cast<DDS::Long>(size);
    dds_message->@(field.name)_.length(length);
@[    end if]@
    for (DDS::ULong i = 0; i < size; ++i) {
@[    if field.type.array_size and not field.type.is_upper_bound]@
      auto & ros_i = ros_message->@(field.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(field.name).data[i];
@[    end if]@
@[    if field.type.type == 'string']@
      const rosidl_generator_c__String * str = &ros_i;
      if (!str) {
        return "string field was not allocated";
      }
      if (str->capacity == 0 || str->capacity <= str->size) {
        return "string capacity not greater than size";
      }
      if (!str->data) {
        return "string data was not allocated";
      }
      if (str->data[str->size] != '\0') {
        return "string not null-terminated";
      }

      dds_message->@(field.name)_[i] = DDS::string_dup(str->data);
@[    elif field.type.type == 'bool']@
      dds_message->@(field.name)_[i] = 1 ? ros_i : 0;
@[    elif field.type.is_primitive_type()]@
      dds_message->@(field.name)_[i] = ros_i;
@[    else]@
      const char * err_msg = @(field.type.pkg_name)__msg__@(field.type.type)__callbacks->convert_ros_to_dds(
        &ros_i, &dds_message->@(field.name)_[i]);
      if (err_msg != 0) {
        return err_msg;
      }
@[    end if]@
    }
@[  elif field.type.type == 'string']@
    const rosidl_generator_c__String * str = &ros_message->@(field.name);
    if (!str) {
      return "string field was not allocated";
    }
    if (str->capacity == 0 || str->capacity <= str->size) {
      return "string capacity not greater than size";
    }
    if (!str->data) {
      return "string data was not allocated";
    }
    if (str->data[str->size] != '\0') {
      return "string not null-terminated";
    }

    dds_message->@(field.name)_ = DDS::string_dup(str->data);
@[  elif field.type.is_primitive_type()]@
    dds_message->@(field.name)_ = ros_message->@(field.name);
@[  else]@
    const char * err_msg = @(field.type.pkg_name)__msg__@(field.type.type)__callbacks->convert_ros_to_dds(
      &ros_message->@(field.name), &dds_message->@(field.name)_);
    if (err_msg != 0) {
      return err_msg;
    }
@[  end if]@
  }

@[end for]@
  return 0;
}

static const char *
publish(void * dds_data_writer, const void * ros_message)
{
  if (!dds_data_writer) {
    return "data writer handle is null";
  }
  if (!ros_message) {
    return "ros message handle is null";
  }

  DDS::DataWriter * topic_writer = static_cast<DDS::DataWriter *>(dds_data_writer);

  __dds_msg_type dds_message;
  const char * err_msg = convert_ros_to_dds(ros_message, &dds_message);
  if (err_msg != 0) {
    return err_msg;
  }

  @(__dds_msg_type_prefix)DataWriter * data_writer =
    @(__dds_msg_type_prefix)DataWriter::_narrow(topic_writer);
  DDS::ReturnCode_t status = data_writer->write(dds_message, DDS::HANDLE_NIL);
@[for field in spec.fields]@
@[  if field.type.type == 'string']@
@[    if field.type.is_array]@
  {
@[      if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[      else]@
    size_t size = dds_message.@(field.name)_.length();
@[      end if]@
    for (DDS::ULong i = 0; i < size; ++i) {
      // This causes the DDS::String_mgr to release the given c string without freeing it.
      dds_message.@(field.name)_[i]._retn();
    }
  }
@[    else]@
  // This causes the DDS::String_mgr to release the given c string without freeing it.
  dds_message.@(field.name)_._retn();
@[    end if]@
@[  end if]@
@[end for]@
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

static const char *
convert_dds_to_ros(const void * untyped_dds_message, void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    return "ros message handle is null";
  }
  if (!untyped_dds_message) {
    return "dds message handle is null";
  }
  const __dds_msg_type * dds_message = static_cast<const __dds_msg_type *>(untyped_dds_message);
  __ros_msg_type * ros_message = static_cast<__ros_msg_type *>(untyped_ros_message);
@[if not spec.fields]@
  (void)dds_message;
  (void)ros_message;
  return 0;  // No fields is a no-op.
@[end if]@
@[for field in spec.fields]@
  // Field name: @(field.name)
  {
@[  if field.type.is_array]@
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
@{
if field.type.type == 'string':
    array_init = 'rosidl_generator_c__String__Array__init'
    array_fini = 'rosidl_generator_c__String__Array__fini'
elif field.type.is_primitive_type():
    array_init = 'rosidl_generator_c__{field.type.type}__Array__init'.format(**locals())
    array_fini = 'rosidl_generator_c__{field.type.type}__Array__fini'.format(**locals())
else:
    array_init = '{field.type.pkg_name}__msg__{field.type.type}__Array__init'.format(**locals())
    array_fini = '{field.type.pkg_name}__msg__{field.type.type}__Array__fini'.format(**locals())
}@
    size_t size = dds_message->@(field.name)_.length();
    if (ros_message->@(field.name).data) {
      @(array_fini)(&ros_message->@(field.name));
    }
    if (!@(array_init)(&ros_message->@(field.name), size)) {
      return "failed to create array for field '@(field.name)'";
    }
@[    end if]@
    for (DDS::ULong i = 0; i < size; i++) {
@[    if field.type.array_size and not field.type.is_upper_bound]@
      auto & ros_i = ros_message->@(field.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(field.name).data[i];
@[    end if]@
@[    if field.type.type == 'bool']@
      ros_i = (dds_message->@(field.name)_[i] != 0);
@[    elif field.type.type == 'string']@
      if (!ros_i.data) {
        rosidl_generator_c__String__init(&ros_i);
      }
      bool succeeded = rosidl_generator_c__String__assign(
        &ros_i,
        dds_message->@(field.name)_[i]);
      if (!succeeded) {
        return "failed to assign string into field '@(field.name)'";
      }
@[    elif field.type.is_primitive_type()]@
      ros_i = dds_message->@(field.name)_[i];
@[    else]@
      const rosidl_message_type_support_t * ts =
        ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_opensplice_c, @(field.type.pkg_name), msg, @(field.type.type))();
      const message_type_support_callbacks_t * callbacks =
        static_cast<const message_type_support_callbacks_t *>(ts->data);
      callbacks->convert_dds_to_ros(&dds_message->@(field.name)_[i], &ros_i);
@[    end if]@
    }
@[  elif field.type.type == 'string']@
    if (!ros_message->@(field.name).data) {
      rosidl_generator_c__String__init(&ros_message->@(field.name));
    }
    bool succeeded = rosidl_generator_c__String__assign(
      &ros_message->@(field.name),
      dds_message->@(field.name)_);
    if (!succeeded) {
      return "failed to assign string into field '@(field.name)'";
    }
@[  elif field.type.is_primitive_type()]@
    ros_message->@(field.name) = dds_message->@(field.name)_@(' == TRUE' if field.type.type == 'bool' else '');
@[  else]@
    const rosidl_message_type_support_t * ts =
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_opensplice_c, @(field.type.pkg_name), msg, @(field.type.type))();
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(ts->data);
    callbacks->convert_dds_to_ros(&dds_message->@(field.name)_, &ros_message->@(field.name));
@[  end if]@
  }

@[end for]@
  return 0;
}

static const char *
take(
  void * dds_data_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle)
{
  if (untyped_ros_message == 0) {
    return "invalid ros message pointer";
  }

  DDS::DataReader * topic_reader = static_cast<DDS::DataReader *>(dds_data_reader);

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
    errs = convert_dds_to_ros(&dds_messages[0], untyped_ros_message);
    if (errs != 0) {
      goto finally;
    }
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
@
@# // Collect the callback functions and provide a function to get the type support struct.

static message_type_support_callbacks_t __callbacks = {
  "@(pkg)",  // package_name
  "@(msg)",  // message_name
  register_type,  // register_type
  publish,  // publish
  take,  // take
  convert_ros_to_dds,  // convert_ros_to_dds
  convert_dds_to_ros,  // convert_dds_to_ros
};

static rosidl_message_type_support_t __type_support = {
  rosidl_typesupport_opensplice_c__identifier,
  &__callbacks,  // data
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_opensplice_c, @(pkg), @(subfolder), @(msg))() {
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif
