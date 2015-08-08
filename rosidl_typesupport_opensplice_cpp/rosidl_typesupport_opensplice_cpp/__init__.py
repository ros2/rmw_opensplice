# Copyright 2014 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import subprocess

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import extract_message_types
from rosidl_cmake import get_newest_modification_time
from rosidl_parser import parse_message_file
from rosidl_parser import parse_service_file
from rosidl_parser import validate_field_types


def generate_dds_opensplice_cpp(
    pkg_name, dds_interface_files, dds_interface_base_path, deps, output_basepath, idl_pp
):
    include_dirs = [dds_interface_base_path]
    for dep in deps:
        # only take the first : for separation, as Windows follows with a C:\
        dep_parts = dep.split(':', 1)
        assert len(dep_parts) == 2, "The dependency '%s' must contain a double colon" % dep
        idl_path = dep_parts[1]
        idl_base_path = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.normpath(idl_path))))
        if idl_base_path not in include_dirs:
            include_dirs.append(idl_base_path)
    if 'OSPL_TMPL_PATH' in os.environ:
        include_dirs.append(os.environ['OSPL_TMPL_PATH'])

    for idl_file in dds_interface_files:
        assert os.path.exists(idl_file), 'Could not find IDL file: ' + idl_file

        # get two level of parent folders for idl file
        folder = os.path.dirname(idl_file)
        parent_folder = os.path.dirname(folder)
        output_path = os.path.join(
            output_basepath,
            os.path.basename(parent_folder),
            os.path.basename(folder))
        try:
            os.makedirs(output_path)
        except FileExistsError:
            pass

        cmd = [idl_pp]
        for include_dir in include_dirs:
            cmd += ['-I', include_dir]
        cmd += [
            '-S',
            '-l', 'cpp',
            '-d', output_path,
            idl_file
        ]
        if os.name == 'nt':
            cmd[-1:-1] = [
                '-P',
                'ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC_%s,%s' %
                (pkg_name, '%s/msg/dds_opensplice/visibility_control.h' % pkg_name)]
        subprocess.check_call(cmd)

        # modify generated code to compile with Visual Studio 2015
        msg_name = os.path.splitext(os.path.basename(idl_file))[0]
        dcps_impl_h_filename = os.path.join(output_path, '%sDcps_impl.h' % msg_name)
        _modify(dcps_impl_h_filename, msg_name, _copy_constructor_and_assignment_operator)

    return 0


def _modify(filename, msg_name, callback):
    with open(filename, 'r') as h:
        lines = h.read().split('\n')
    modified = callback(lines, msg_name)
    if modified:
        with open(filename, 'w') as h:
            h.write('\n'.join(lines))


def _copy_constructor_and_assignment_operator(lines, msg_name):
    for i, line in enumerate(lines):
        if line.strip() == 'public:':
            new_lines = [
                '%sTypeSupportFactory(const %sTypeSupportFactory & o) = delete;' %
                (msg_name, msg_name),
                '%sTypeSupportFactory & operator=(const %sTypeSupportFactory & o) = delete;' %
                (msg_name, msg_name),
            ]
            lines[i + 1:i + 1] = [' ' * 16 + l for l in new_lines]
            return lines
    assert False, 'Failed to find insertion point'


def generate_typesupport_opensplice_cpp(args):
    template_dir = args['template_dir']
    mapping_msgs = {
        os.path.join(template_dir, 'msg__type_support.hpp.template'): '%s__type_support.hpp',
        os.path.join(template_dir, 'msg__type_support.cpp.template'): '%s__type_support.cpp',
    }

    mapping_srvs = {
        os.path.join(template_dir, 'srv__type_support.cpp.template'):
        '%s__type_support.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file

    for template_file in mapping_srvs.keys():
        assert os.path.exists(template_file), 'Could not find template: ' + template_file

    pkg_name = args['package_name']
    known_msg_types = extract_message_types(
        pkg_name, args['ros_interface_files'], args.get('ros_interface_dependencies', []))

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }
    # generate_dds_opensplice_cpp() and therefore the make target depend on the additional files
    # therefore they must be listed here even if the generated type support files are independent
    latest_target_timestamp = get_newest_modification_time(
        args['target_dependencies'] + args.get('additional_files', []))

    for idl_file in args['ros_interface_files']:
        extension = os.path.splitext(idl_file)[1]
        if extension == '.msg':
            spec = parse_message_file(pkg_name, idl_file)
            validate_field_types(spec, known_msg_types)
            subfolder = os.path.basename(os.path.dirname(idl_file))
            for template_file, generated_filename in mapping_msgs.items():
                generated_file = os.path.join(
                    args['output_dir'], subfolder, 'dds_opensplice', generated_filename %
                    convert_camel_case_to_lower_case_underscore(spec.base_type.type))

                data = {'spec': spec, 'subfolder': subfolder}
                data.update(functions)
                expand_template(
                    template_file, data, generated_file,
                    minimum_timestamp=latest_target_timestamp)

        elif extension == '.srv':
            spec = parse_service_file(pkg_name, idl_file)
            validate_field_types(spec, known_msg_types)
            for template_file, generated_filename in mapping_srvs.items():
                generated_file = os.path.join(
                    args['output_dir'], 'srv', 'dds_opensplice', generated_filename %
                    convert_camel_case_to_lower_case_underscore(spec.srv_name))

                data = {'spec': spec}
                data.update(functions)
                expand_template(
                    template_file, data, generated_file,
                    minimum_timestamp=latest_target_timestamp)

    return 0
