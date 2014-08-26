from rosidl_generator_dds_idl import get_include_directives as \
    get_default_include_directives
from rosidl_generator_dds_idl import msg_type_to_idl as \
    default_msg_type_to_idl
from rosidl_generator_dds_idl import _msg_type_to_idl


def get_include_directives(spec, subfolders):
    includes = get_default_include_directives(spec, subfolders)

    # remove directives for builtin_msgs
    includes = [i for i in includes if '"builtin_msgs/' not in i]

    # prepend directives to include opensplice specific idl file
    for field in spec.fields:
        if field.type.pkg_name == 'builtin_msgs':
            includes[:0] = [
                '#ifndef OSPL_IDL_COMPILER',
                '#include <dds_dcps.idl>',
                '#endif']
            break
    return includes


def msg_type_to_idl(type_):
    if type_.pkg_name == 'builtin_msgs':
        idl_type = 'DDS::%s_t' % type_.type
        return _msg_type_to_idl(type_, idl_type)
    return default_msg_type_to_idl(type_)
