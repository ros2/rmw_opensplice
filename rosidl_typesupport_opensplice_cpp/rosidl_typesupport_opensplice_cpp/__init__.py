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
            '-N',
            '-d', output_path,
            idl_file
        ]
        if os.name == 'nt':
            cmd[-1:-1] = [
                '-P',
                'ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_PUBLIC_%s,%s' % (
                    pkg_name,
                    '%s/msg/rosidl_typesupport_opensplice_cpp__visibility_control.h' % pkg_name)]
        subprocess.check_call(cmd)

        # modify generated code to
        # remove path information of the building machine as well as timestamps
        msg_name = os.path.splitext(os.path.basename(idl_file))[0]
        idl_path = os.path.join(
            pkg_name, os.path.basename(parent_folder), os.path.basename(folder),
            os.path.basename(idl_file))
        h_filename = os.path.join(output_path, '%s.h' % msg_name)
        _modify(h_filename, msg_name, _replace_path_and_timestamp, idl_path=idl_path)
        cpp_filename = os.path.join(output_path, '%s.cpp' % msg_name)
        _modify(cpp_filename, msg_name, _replace_path_and_timestamp, idl_path=idl_path)
        dcps_h_filename = os.path.join(output_path, '%sDcps.h' % msg_name)
        _modify(dcps_h_filename, msg_name, _replace_path_and_timestamp, idl_path=idl_path)
        dcps_cpp_filename = os.path.join(output_path, '%sDcps.cpp' % msg_name)
        _modify(dcps_cpp_filename, msg_name, _replace_path_and_timestamp, idl_path=idl_path)

    return 0


def _modify(filename, msg_name, callback, idl_path=None):
    with open(filename, 'r') as h:
        lines = h.read().split('\n')
    modified = callback(lines, msg_name, idl_path=idl_path)
    if modified:
        with open(filename, 'w') as h:
            h.write('\n'.join(lines))


def _replace_path_and_timestamp(lines, msg_name, idl_path):
    found_source = False
    for i, line in enumerate(lines):
        if line.startswith('//  Source: '):
            assert not found_source, "More than one '// Source: ' line was found"
            found_source = True
            lines[i] = '//  Source: ' + idl_path
            continue
        if line.startswith('//  Generated: '):
            assert found_source, "No '// Source: ' line was found before"
            lines[i] = '//  Generated: timestamp removed to make the build reproducible'
            break
    else:
        assert False, "Failed to find '// Generated: ' line"
    return lines


def generate_typesupport_opensplice_cpp(args):
    template_dir = args['template_dir']
    mapping_msgs = {
        os.path.join(template_dir, 'msg__rosidl_typesupport_opensplice_cpp.hpp.em'):
        '%s__rosidl_typesupport_opensplice_cpp.hpp',
        os.path.join(template_dir, 'msg__type_support.cpp.em'):
        '%s__type_support.cpp',
    }

    mapping_srvs = {
        os.path.join(template_dir, 'srv__rosidl_typesupport_opensplice_cpp.hpp.em'):
        '%s__rosidl_typesupport_opensplice_cpp.hpp',
        os.path.join(template_dir, 'srv__type_support.cpp.em'):
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
                generated_file = os.path.join(args['output_dir'], subfolder)
                if generated_filename.endswith('.cpp'):
                    generated_file = os.path.join(generated_file, 'dds_opensplice')
                generated_file = os.path.join(
                    generated_file, generated_filename %
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
                generated_file = os.path.join(args['output_dir'], 'srv')
                if generated_filename.endswith('.cpp'):
                    generated_file = os.path.join(generated_file, 'dds_opensplice')
                generated_file = os.path.join(
                    generated_file, generated_filename %
                    convert_camel_case_to_lower_case_underscore(spec.srv_name))

                data = {'spec': spec}
                data.update(functions)
                expand_template(
                    template_file, data, generated_file,
                    minimum_timestamp=latest_target_timestamp)

    return 0
