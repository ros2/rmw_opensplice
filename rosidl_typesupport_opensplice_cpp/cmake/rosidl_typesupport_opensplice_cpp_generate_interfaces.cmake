# Copyright 2014-2015 Open Source Robotics Foundation, Inc.
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

set(_target_suffix "__rosidl_typesupport_opensplice_cpp")

# avoid generating any opensplice specific stuff for builtin_msgs
if(NOT "${PROJECT_NAME} " STREQUAL "builtin_msgs ")

rosidl_generate_dds_interfaces(
  ${rosidl_generate_interfaces_TARGET}__dds_opensplice_idl
  IDL_FILES ${rosidl_generate_interfaces_IDL_FILES}
  DEPENDENCY_PACKAGE_NAMES ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}
  OUTPUT_SUBFOLDERS "dds_opensplice"
  EXTENSION "rosidl_typesupport_opensplice_cpp.rosidl_generator_dds_idl_extension"
)

set(_dds_idl_files "")
set(_dds_idl_base_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_dds_idl")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  get_filename_component(_name "${_idl_file}" NAME_WE)
  if("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    list(APPEND _dds_idl_files
      "${_dds_idl_base_path}/${PROJECT_NAME}/${_parent_folder}/dds_opensplice/${_name}_.idl")
  elseif("${_extension} " STREQUAL ".srv ")
    list(APPEND _dds_idl_files
      "${_dds_idl_base_path}/${PROJECT_NAME}/srv/dds_opensplice/Sample_${_name}_Request_.idl")
    list(APPEND _dds_idl_files
      "${_dds_idl_base_path}/${PROJECT_NAME}/srv/dds_opensplice/Sample_${_name}_Response_.idl")
  endif()
endforeach()

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp/${PROJECT_NAME}")
set(_generated_msg_files "")
set(_generated_srv_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  get_filename_component(_msg_name "${_idl_file}" NAME_WE)
  string_camel_case_to_lower_case_underscore("${_msg_name}" _header_name)
  if("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    list(APPEND _generated_msg_files
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_.h"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_.cpp"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps.h"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps.cpp"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps_impl.h"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps_impl.cpp"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_SplDcps.h"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_SplDcps.cpp"
      "${_output_path}/${_parent_folder}/dds_opensplice/ccpp_${_msg_name}_.h"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_header_name}__type_support.hpp"
      "${_output_path}/${_parent_folder}/dds_opensplice/${_header_name}__type_support.cpp")
  elseif("${_extension} " STREQUAL ".srv ")
    list(APPEND _generated_srv_files "${_output_path}/srv/dds_opensplice/${_header_name}__type_support.cpp")

    foreach(_suffix "_Request" "_Response")
      list(APPEND _generated_srv_files
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_.h"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_.cpp"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps.h"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps.cpp"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps_impl.h"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps_impl.cpp"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_SplDcps.h"
        "${_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_SplDcps.cpp"
        "${_output_path}/srv/dds_opensplice/ccpp_Sample_${_msg_name}${_suffix}_.h")
    endforeach()
  else()
    message(FATAL_ERROR "Interface file with unknown extension: ${_idl_file}")
  endif()
endforeach()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
    get_filename_component(_extension "${_idl_file}" EXT)
    if("${_extension} " STREQUAL ".msg ")
      get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
      get_filename_component(_parent_folder "${_parent_folder}" NAME)
      # ignore builtin_msgs since it does not have any idl files
      if(NOT "${_pkg_name} " STREQUAL "builtin_msgs ")
        get_filename_component(_name "${_idl_file}" NAME_WE)
        set(_abs_idl_file "${${_pkg_name}_DIR}/../${_parent_folder}/dds_opensplice/${_name}_.idl")
        normalize_path(_abs_idl_file "${_abs_idl_file}")
        list(APPEND _dependency_files "${_abs_idl_file}")
      endif()
      set(_abs_idl_file "${${_pkg_name}_DIR}/../${_idl_file}")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
    elseif("${_extension} " STREQUAL ".srv ")
      get_filename_component(_name "${_idl_file}" NAME_WE)

      set(_abs_idl_file "${${_pkg_name}_DIR}/../srv/dds_opensplice/Sample_${_name}_Request_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependency_files "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")

      set(_abs_idl_file "${${_pkg_name}_DIR}/../srv/dds_opensplice/Sample_${_name}_Response_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependency_files "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
    endif()
  endforeach()
endforeach()

set(_dds_idl_files_file "${_output_path}/dds_idl_files.txt")
string(REPLACE ";" "\n" _dds_idl_files_lines "${_dds_idl_files}")
file(WRITE "${_dds_idl_files_file}" ${_dds_idl_files_lines})

add_custom_command(
  OUTPUT ${_generated_msg_files} ${_generated_srv_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_opensplice_cpp_BIN}
  --pkg-name ${PROJECT_NAME}
  --ros-interface-files ${rosidl_generate_interfaces_IDL_FILES}
  --dds-interface-files-file ${_dds_idl_files_file}
  --dds-interface-base-path ${_dds_idl_base_path}
  --deps ${_dependencies}
  --output-dir "${_output_path}"
  --idl-pp "${OpenSplice_IDLPP}"
  --template-dir ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}
  DEPENDS
  ${rosidl_typesupport_opensplice_cpp_BIN}
  ${rosidl_typesupport_opensplice_cpp_GENERATOR_FILES}
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/msg__type_support.hpp.template
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/msg__type_support.cpp.template
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/srv__type_support.cpp.template
  ${_dds_idl_files}
  ${_dependency_files}
  COMMENT "Generating C++ type support for PrismTech OpenSplice"
  VERBATIM
)

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}__dds_opensplice_idl
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)

if(NOT "${_generated_msg_files} " STREQUAL " ")
  install(
    FILES ${_generated_msg_files}
    DESTINATION "include/${PROJECT_NAME}/msg/dds_opensplice"
  )
endif()
if(NOT "${_generated_srv_files} " STREQUAL " ")
  install(
    FILES ${_generated_srv_files}
    DESTINATION "include/${PROJECT_NAME}/srv/dds_opensplice"
  )
endif()

else()

  # generate specific type support code for the builtin_msgs package
  set(_generated_msg_files
    ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/duration__type_support.cpp
    ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/time__type_support.cpp
  )
  set(_generated_srv_files)

endif()

link_directories(${OpenSplice_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED
  ${_generated_msg_files} ${_generated_srv_files})
if(WIN32)
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_BUILDING_DLL")
endif()
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp
  "${_output_path}/msg/dds_opensplice"
  "${_output_path}/srv/dds_opensplice"
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_msg_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/msg/dds_opensplice")
  set(_srv_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/srv/dds_opensplice")
  normalize_path(_msg_include_dir "${_msg_include_dir}")
  normalize_path(_srv_include_dir "${_srv_include_dir}")
  target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    "${_msg_include_dir}"
    "${_srv_include_dir}"
  )
  ament_target_dependencies(
    ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ${_pkg_name})
endforeach()
ament_target_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "OpenSplice"
  "rosidl_typesupport_opensplice_cpp")

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__cpp
)

install(
  TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${OpenSplice_LIBRARIES})

ament_export_include_directories(include)
