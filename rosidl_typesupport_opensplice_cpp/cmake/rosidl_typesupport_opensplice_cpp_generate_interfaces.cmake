message(" - rosidl_typesupport_opensplice_cpp_generate_interfaces.cmake")
message("   - target: ${rosidl_generate_interfaces_TARGET}")
message("   - interface files: ${rosidl_generate_interfaces_IDL_FILES}")
message("   - dependency package names: ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}")

# avoid generating any opensplice specific stuff for builtin_msgs
if(NOT "${PROJECT_NAME}" STREQUAL "builtin_msgs")

rosidl_generate_dds_interfaces(
  ${rosidl_generate_interfaces_TARGET}__dds_opensplice_idl
  IDL_FILES ${rosidl_generate_interfaces_IDL_FILES}
  DEPENDENCY_PACKAGE_NAMES ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}
  OUTPUT_SUBFOLDERS "dds_opensplice"
  EXTENSION "rosidl_typesupport_opensplice_cpp.rosidl_generator_dds_idl_extension"
)

set(_dds_idl_files "")
set(_dds_idl_base_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_dds_idl")
set(_dds_idl_path "${_dds_idl_base_path}/${PROJECT_NAME}/dds_opensplice")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(name "${_idl_file}" NAME_WE)
  list(APPEND _dds_idl_files "${_dds_idl_path}/${name}_.idl")
endforeach()

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp/${PROJECT_NAME}/dds_opensplice")
set(_generated_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(name "${_idl_file}" NAME_WE)
  list(APPEND _generated_files "${_output_path}/${name}_.h")
  list(APPEND _generated_files "${_output_path}/${name}_.cpp")
  list(APPEND _generated_files "${_output_path}/${name}_Dcps.h")
  list(APPEND _generated_files "${_output_path}/${name}_Dcps.cpp")
  list(APPEND _generated_files "${_output_path}/${name}_Dcps_impl.h")
  list(APPEND _generated_files "${_output_path}/${name}_Dcps_impl.cpp")
  list(APPEND _generated_files "${_output_path}/${name}_SplDcps.h")
  list(APPEND _generated_files "${_output_path}/${name}_SplDcps.cpp")
  list(APPEND _generated_files "${_output_path}/ccpp_${name}_.h")
  list(APPEND _generated_files "${_output_path}/${name}_TypeSupport.cpp")
endforeach()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
    # ignore builtin_msgs since it does not have any idl files
    if(NOT "${_pkg_name}" STREQUAL "builtin_msgs")
      get_filename_component(name "${_idl_file}" NAME_WE)
      set(_abs_idl_file "${${_pkg_name}_DIR}/../dds_opensplice/${name}_.idl")
      list(APPEND _dependency_files "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
    endif()
  endforeach()
endforeach()

message("   - generated files: ${_generated_files}")
message("   - dependencies: ${_dependencies}")

add_custom_command(
  OUTPUT ${_generated_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_opensplice_cpp_BIN}
  --pkg-name ${PROJECT_NAME}
  --ros-interface-files ${rosidl_generate_interfaces_IDL_FILES}
  --dds-interface-files ${_dds_idl_files}
  --dds-interface-base-path ${_dds_idl_base_path}
  --deps ${_dependencies}
  --output-dir "${_output_path}"
  --idl-pp "${OPENSPLICE_IDLPP}"
  --template-dir ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}
  DEPENDS
  ${rosidl_typesupport_opensplice_cpp_BIN}
  ${rosidl_typesupport_opensplice_cpp_DIR}/../../../${PYTHON_INSTALL_DIR}/rosidl_typesupport_opensplice_cpp/__init__.py
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/msg_TypeSupport.cpp.template
  ${_dds_idl_files}
  ${_dependency_files}
  COMMENT "Generating C++ type support for PrismTech OpenSplice"
  VERBATIM
)

set(_target_suffix "__dds_opensplice_cpp")

add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED ${_generated_files})
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC ${OPENSPLICE_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp
  ${rosidl_typesupport_opensplice_cpp_INCLUDE_DIRS}
  ${rosidl_generator_cpp_INCLUDE_DIRS}
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    ${${_pkg_name}_INCLUDE_DIRS}
    ${${_pkg_name}_DIR}/../../../include/${_pkg_name}/dds_opensplice
  )
  target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ${${_pkg_name}_LIBRARIES})
endforeach()
target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${OPENSPLICE_LIBRARIES})

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__cpp
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}__dds_opensplice_idl
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)

install(
  FILES ${_generated_files}
  DESTINATION "include/${PROJECT_NAME}/dds_opensplice"
)
install(
  TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  DESTINATION "lib"
)

ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${OPENSPLICE_LIBRARIES})

endif()

ament_export_include_directories(include)
