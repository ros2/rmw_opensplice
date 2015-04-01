import em
import os
import subprocess

from rosidl_parser import parse_message_file, parse_service_file


def generate_dds_opensplice_cpp(
        pkg_name, dds_interface_files, dds_interface_base_path, deps, output_dir, idl_pp):
    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass

    include_dirs = [dds_interface_base_path]
    for dep in deps:
        dep_parts = dep.split(':')
        assert(len(dep_parts) == 2)
        idl_path = dep_parts[1]
        idl_base_path = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.normpath(idl_path))))
        if idl_base_path not in include_dirs:
            include_dirs.append(idl_base_path)

    for idl_file in dds_interface_files:
        generated_file = os.path.join(
            output_dir,
            os.path.splitext(os.path.basename(idl_file))[0] + '.h/cpp')
        print('Generating: %s' % generated_file)

        cmd = [idl_pp]
        for include_dir in include_dirs:
            cmd += ['-I', include_dir]
        cmd += [
            '-S',
            '-l', 'cpp',
            '-o', 'dds-types',
            '-d', output_dir,
            idl_file
        ]
        subprocess.check_call(cmd)

    return 0


def generate_typesupport_opensplice_cpp(
    pkg_name, ros_interface_files, deps, output_dir, template_dir
):
    mapping_msgs = {
        os.path.join(template_dir, 'msg_TypeSupport.h.template'): '%s_TypeSupport.h',
        os.path.join(template_dir, 'msg_TypeSupport.cpp.template'): '%s_TypeSupport.cpp',
    }

    mapping_srvs = {
        os.path.join(template_dir, 'srv_ServiceTypeSupport.cpp.template'):
        '%s_ServiceTypeSupport.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert(os.path.exists(template_file))

    for template_file in mapping_srvs.keys():
        assert(os.path.exists(template_file))

    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass

    for idl_file in ros_interface_files:
        print(pkg_name, idl_file)
        filename, extension = os.path.splitext(idl_file)
        if extension == '.msg':
            spec = parse_message_file(pkg_name, idl_file)
            for template_file, generated_filename in mapping_msgs.items():
                generated_file = os.path.join(output_dir, generated_filename % spec.base_type.type)
                print('Generating MESSAGE: %s' % generated_file)

                try:
                    # TODO only touch generated file if its content actually changes
                    ofile = open(generated_file, 'w')
                    # TODO reuse interpreter
                    interpreter = em.Interpreter(
                        output=ofile,
                        options={
                            em.RAW_OPT: True,
                            em.BUFFERED_OPT: True,
                        },
                        globals={'spec': spec},
                    )
                    interpreter.file(open(template_file))
                    interpreter.shutdown()
                except Exception:
                    os.remove(generated_file)
                    raise
        elif extension == '.srv':
            spec = parse_service_file(pkg_name, idl_file)
            for template_file, generated_filename in mapping_srvs.items():
                generated_file = os.path.join(output_dir, generated_filename % spec.srv_name)
                print('Generating SERVICE: %s' % generated_file)

                try:
                    # TODO only touch generated file if its content actually changes
                    ofile = open(generated_file, 'w')
                    # TODO reuse interpreter
                    interpreter = em.Interpreter(
                        output=ofile,
                        options={
                            em.RAW_OPT: True,
                            em.BUFFERED_OPT: True,
                        },
                        globals={'spec': spec},
                    )
                    interpreter.file(open(template_file))
                    interpreter.shutdown()
                except Exception:
                    os.remove(generated_file)
                    raise

    return 0
