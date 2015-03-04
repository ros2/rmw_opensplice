cmake_minimum_required(VERSION 2.8.3)

project(rmw_opensplice_cpp)

find_package(ament_cmake REQUIRED)

find_package(opensplice_cmake_module REQUIRED)
find_package(OpenSplice MODULE)
if(NOT OpenSplice_FOUND)
  ament_package()
  message(WARNING "Could not find PrismTech OpenSplice - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rmw REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
find_package(rosidl_typesupport_opensplice_cpp REQUIRED)

ament_export_definitions(${OpenSplice_DEFINITIONS})
ament_export_dependencies(rmw rosidl_generator_cpp rosidl_typesupport_opensplice_cpp)
ament_export_include_directories(include/${PROJECT_NAME}/impl ${OpenSplice_INCLUDE_DIRS})

link_directories(${OpenSplice_LIBRARY_DIRS})
add_library(rmw_opensplice_cpp SHARED src/functions.cpp)
ament_target_dependencies(rmw_opensplice_cpp
  "rmw"
  "rosidl_generator_cpp"
  "rosidl_typesupport_opensplice_cpp"
  "OpenSplice")
configure_rmw_library(rmw_opensplice_cpp)

ament_export_libraries(rmw_opensplice_cpp ${OpenSplice_LIBRARIES})

register_rmw_implementation()

ament_package()

install(
  TARGETS rmw_opensplice_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
