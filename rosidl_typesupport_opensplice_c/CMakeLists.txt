cmake_minimum_required(VERSION 2.8.3)

project(rosidl_typesupport_opensplice_c)

find_package(ament_cmake REQUIRED)

find_package(rosidl_typesupport_opensplice_cpp REQUIRED)

ament_export_dependencies(rosidl_typesupport_opensplice_cpp)
# The reason the impl folder is exported is that it contains the implementation
# for the get_*_type_support_handle functions and defines the opensplice
# specific version of these functions.
ament_export_include_directories(include include/${PROJECT_NAME}/impl)

ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
