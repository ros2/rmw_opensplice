# Copyright 2016 Open Source Robotics Foundation, Inc.
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

set(_target_suffix "__rosidl_typesupport_opensplice_c")

# avoid generating any opensplice specific stuff for builtin_interfaces
if(NOT "${PROJECT_NAME} " STREQUAL "builtin_interfaces ")

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

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_c/${PROJECT_NAME}")
set(_dds_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp/${PROJECT_NAME}")
set(_generated_msg_files "")
set(_generated_external_msg_files "")
set(_generated_srv_files "")
set(_generated_external_srv_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  get_filename_component(_msg_name "${_idl_file}" NAME_WE)
  string_camel_case_to_lower_case_underscore("${_msg_name}" _header_name)
  if("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    list(APPEND _generated_external_msg_files
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_.h"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_.cpp"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps.h"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps.cpp"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps_impl.h"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_Dcps_impl.cpp"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_SplDcps.h"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/${_msg_name}_SplDcps.cpp"
      "${_dds_output_path}/${_parent_folder}/dds_opensplice/ccpp_${_msg_name}_.h")
    list(APPEND _generated_msg_files
      "${_output_path}/${_parent_folder}/dds_opensplice_c/${_header_name}__type_support_c.cpp")
  elseif("${_extension} " STREQUAL ".srv ")
    list(APPEND _generated_srv_files "${_output_path}/srv/dds_opensplice_c/${_header_name}__type_support_c.cpp")

    foreach(_suffix "_Request" "_Response")
      list(APPEND _generated_external_srv_files
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_.h"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_.cpp"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps.h"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps.cpp"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps_impl.h"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_Dcps_impl.cpp"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_SplDcps.h"
        "${_dds_output_path}/srv/dds_opensplice/Sample_${_msg_name}${_suffix}_SplDcps.cpp"
        "${_dds_output_path}/srv/dds_opensplice/ccpp_Sample_${_msg_name}${_suffix}_.h")
    endforeach()
  else()
    message(FATAL_ERROR "Interface file with unknown extension: ${_idl_file}")
  endif()
endforeach()

# If not on Windows, disable some warnings with OpenSplice's generated code
if(NOT WIN32)
  set(_opensplice_compile_flags)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(_opensplice_compile_flags
      "-Wno-unused-but-set-variable"
    )
  endif()
  if(NOT "${_opensplice_compile_flags} " STREQUAL " ")
    string(REPLACE ";" " " _opensplice_compile_flags "${_opensplice_compile_flags}")
    foreach(_gen_file ${_generated_external_msg_files} ${_generated_external_srv_files})
      set_source_files_properties("${_gen_file}"
        PROPERTIES COMPILE_FLAGS ${_opensplice_compile_flags})
    endforeach()
  endif()
endif()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
    get_filename_component(_extension "${_idl_file}" EXT)
    if("${_extension} " STREQUAL ".msg ")
      get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
      get_filename_component(_parent_folder "${_parent_folder}" NAME)
      # ignore builtin_interfaces since it does not have any idl files
      if(NOT "${_pkg_name} " STREQUAL "builtin_interfaces ")
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

set(target_dependencies
  "${rosidl_typesupport_opensplice_c_BIN}"
  ${rosidl_typesupport_opensplice_c_GENERATOR_FILES}
  "${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/msg__type_support_c.cpp.em"
  "${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/msg__type_support.h.em"
  "${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/srv__type_support_c.cpp.em"
  ${_dependency_files})
foreach(dep ${target_dependencies})
  if(NOT EXISTS "${dep}")
    message(FATAL_ERROR "Target dependency '${dep}' does not exist")
  endif()
endforeach()

set(generator_arguments_file "${CMAKE_BINARY_DIR}/rosidl_typesupport_opensplice_c__arguments.json")
rosidl_write_generator_arguments(
  "${generator_arguments_file}"
  PACKAGE_NAME "${PROJECT_NAME}"
  ROS_INTERFACE_FILES "${rosidl_generate_interfaces_IDL_FILES}"
  ROS_INTERFACE_DEPENDENCIES "${_dependencies}"
  OUTPUT_DIR "${_output_path}"
  TEMPLATE_DIR "${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}"
  TARGET_DEPENDENCIES ${target_dependencies}
  ADDITIONAL_FILES ${_dds_idl_files}
)

add_custom_command(
  OUTPUT
  ${_generated_msg_files}
  ${_generated_srv_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_opensplice_c_BIN}
  --generator-arguments-file "${generator_arguments_file}"
  DEPENDS
  ${target_dependencies}
  ${_dds_idl_files}
  ${_generated_external_msg_files}
  ${_generated_external_srv_files}
  COMMENT "Generating C type support for PrismTech OpenSplice"
  VERBATIM
)

# generate header to switch between export and import for a specific package on Windows
set(_visibility_control_file
  "${_output_path}/msg/dds_opensplice_c/visibility_control.h")
configure_file(
  "${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/visibility_control.h.in"
  "${_visibility_control_file}"
  @ONLY
)
list(APPEND _generated_msg_files "${_visibility_control_file}")

if(NOT rosidl_generate_interfaces_SKIP_INSTALL)
  if(NOT "${_generated_msg_files}${_generated_external_msg_files} " STREQUAL " ")
    install(
      FILES ${_generated_msg_files} ${_generated_external_msg_files}
      DESTINATION "include/${PROJECT_NAME}/msg/dds_opensplice_c"
    )
  endif()
  if(NOT "${_generated_srv_files}${_generated_external_srv_files} " STREQUAL " ")
    install(
      FILES ${_generated_srv_files} ${_generated_external_srv_files}
      DESTINATION "include/${PROJECT_NAME}/srv/dds_opensplice_c"
    )
  endif()
  if(
    NOT "${_generated_msg_files}${_generated_external_msg_files} " STREQUAL " " OR
    NOT "${_generated_srv_files}${_generated_external_srv_files} " STREQUAL " "
  )
    ament_export_include_directories(include)
  endif()
endif()

else()

  # generate specific type support code for the builtin_interfaces package
  set(_generated_msg_files
    ${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/duration__type_support_c.cpp
    ${rosidl_typesupport_opensplice_c_TEMPLATE_DIR}/time__type_support_c.cpp
  )
  set(_generated_external_msg_files)
  set(_generated_srv_files)
  set(_generated_external_srv_files)

endif()

link_directories(${OpenSplice_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED
  ${_generated_msg_files}
  ${_generated_external_msg_files}
  ${_generated_srv_files}
  ${_generated_external_srv_files})
if(NOT WIN32)
  set_target_properties(${rosidl_generate_interfaces_TARGET}${_target_suffix} PROPERTIES
    COMPILE_FLAGS "-std=c++11 -Wall -Wextra")
endif()
if(WIN32)
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_TYPESUPPORT_OPENSPLICE_C_BUILDING_DLL")
endif()
# The following still uses CPP because the OpenSplice code which uses it was generated for CPP.
target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PRIVATE "ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_BUILDING_DLL_${PROJECT_NAME}")
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_c
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_c
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp
  "${_output_path}/msg/dds_opensplice_c"
  "${_dds_output_path}/msg/dds_opensplice"
  "${_output_path}/srv/dds_opensplice_c"
)
ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "rmw"
  "rosidl_generator_c"
)
target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__rosidl_typesupport_opensplice_cpp
  ${rosidl_generate_interfaces_TARGET}__rosidl_generator_c)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_msg_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/msg/dds_opensplice_c")
  set(_srv_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/srv/dds_opensplice_c")
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
  "rosidl_typesupport_opensplice_c")
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  # Add a dependency on the .idl -> OpenSplice code which is created in the C++
  # version of this file, in the rosidl_typesupport_opensplice_cpp package.
  ${rosidl_generate_interfaces_TARGET}__dds_opensplice_idl
)

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__cpp
)

if(NOT rosidl_generate_interfaces_SKIP_INSTALL)
  install(
    TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

  ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${OpenSplice_LIBRARIES})
endif()
