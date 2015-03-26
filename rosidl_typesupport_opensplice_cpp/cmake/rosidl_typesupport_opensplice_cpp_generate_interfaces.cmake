message(" - rosidl_typesupport_opensplice_cpp_generate_interfaces.cmake")
message("   - target: ${rosidl_generate_interfaces_TARGET}")
message("   - interface files: ${rosidl_generate_interfaces_IDL_FILES}")
message("   - dependency package names: ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}")

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
set(_dds_idl_path "${_dds_idl_base_path}/${PROJECT_NAME}/dds_opensplice")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  if("${_extension}" STREQUAL ".msg")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _dds_idl_files "${_dds_idl_path}/${name}_.idl")
  elseif("${_extension}" STREQUAL ".srv")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _dds_idl_files "${_dds_idl_path}/Sample${name}Request_.idl")
    list(APPEND _dds_idl_files "${_dds_idl_path}/Sample${name}Response_.idl")
  endif()
endforeach()

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp/${PROJECT_NAME}/dds_opensplice")
set(_generated_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  if("${_extension}" STREQUAL ".msg")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _generated_files
      "${_output_path}/${name}_.h"
      "${_output_path}/${name}_.cpp"
      "${_output_path}/${name}_Dcps.h"
      "${_output_path}/${name}_Dcps.cpp"
      "${_output_path}/${name}_Dcps_impl.h"
      "${_output_path}/${name}_Dcps_impl.cpp"
      "${_output_path}/${name}_SplDcps.h"
      "${_output_path}/${name}_SplDcps.cpp"
      "${_output_path}/ccpp_${name}_.h"
      "${_output_path}/${name}_TypeSupport.h"
      "${_output_path}/${name}_TypeSupport.cpp")
  elseif("${_extension}" STREQUAL ".srv")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _generated_files "${_output_path}/${name}_ServiceTypeSupport.cpp")

    foreach(_suffix Request Response)
      list(APPEND _generated_files
        "${_output_path}/Sample${name}${_suffix}_.h"
        "${_output_path}/Sample${name}${_suffix}_.cpp"
        "${_output_path}/Sample${name}${_suffix}_Dcps.h"
        "${_output_path}/Sample${name}${_suffix}_Dcps.cpp"
        "${_output_path}/Sample${name}${_suffix}_Dcps_impl.h"
        "${_output_path}/Sample${name}${_suffix}_Dcps_impl.cpp"
        "${_output_path}/Sample${name}${_suffix}_SplDcps.h"
        "${_output_path}/Sample${name}${_suffix}_SplDcps.cpp"
        "${_output_path}/ccpp_Sample${name}${_suffix}_.h")
    endforeach()
  endif()
endforeach()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
    get_filename_component(_extension "${_idl_file}" EXT)
    if("${_extension}" STREQUAL ".msg")
      # ignore builtin_msgs since it does not have any idl files
      if(NOT "${_pkg_name} " STREQUAL "builtin_msgs ")
        get_filename_component(name "${_idl_file}" NAME_WE)
        set(_abs_idl_file "${${_pkg_name}_DIR}/../dds_opensplice/${name}_.idl")
        normalize_path(_abs_idl_file "${_abs_idl_file}")
        list(APPEND _dependency_files "${_abs_idl_file}")
        list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
      endif()
    elseif("${_extension}" STREQUAL ".srv")
      get_filename_component(name "${_idl_file}" NAME_WE)

      set(_abs_idl_file "${${_pkg_name}_DIR}/../dds_opensplice/Sample${name}Request_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependency_files "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")

      set(_abs_idl_file "${${_pkg_name}_DIR}/../dds_opensplice/Sample${name}Response_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
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
  --idl-pp "${OpenSplice_IDLPP}"
  --template-dir ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}
  DEPENDS
  ${rosidl_typesupport_opensplice_cpp_BIN}
  ${rosidl_typesupport_opensplice_cpp_GENERATOR_FILES}
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/msg_TypeSupport.h.template
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/msg_TypeSupport.cpp.template
  ${rosidl_typesupport_opensplice_cpp_TEMPLATE_DIR}/srv_ServiceTypeSupport.cpp.template
  ${_dds_idl_files}
  ${_dependency_files}
  COMMENT "Generating C++ type support for PrismTech OpenSplice"
  VERBATIM
)

set(_target_suffix "__dds_opensplice_cpp")

link_directories(${OpenSplice_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED ${_generated_files})
if(WIN32)
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_BUILDING_DLL")
endif()
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_opensplice_cpp
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/dds_opensplice")
  normalize_path(_include_dir "${_include_dir}")
  target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    "${_include_dir}"
  )
  ament_target_dependencies(
    ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ${_pkg_name})
endforeach()
ament_target_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "OpenSplice"
  "rosidl_generator_cpp"
  "rosidl_typesupport_opensplice_cpp")

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
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${OpenSplice_LIBRARIES})

endif()

ament_export_include_directories(include)
