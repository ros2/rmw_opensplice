cmake_minimum_required(VERSION 2.8.3)

project(ros_middleware_opensplice_cpp)

find_package(ament_cmake REQUIRED)
find_package(ros_middleware_interface REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
find_package(rosidl_typesupport_opensplice_cpp REQUIRED)

find_package(opensplice REQUIRED COMPONENTS CXX)

ament_export_definitions(${OPENSPLICE_DEFINITIONS})
ament_export_dependencies(ros_middleware_interface rosidl_generator_cpp rosidl_typesupport_opensplice_cpp)
ament_export_include_directories(include include/${PROJECT_NAME}/impl ${OPENSPLICE_INCLUDE_DIRS})
ament_export_libraries(ros_middleware_opensplice_cpp dl pthread)

register_ros_middleware_implementation()

ament_package()

include_directories(include ${ros_middleware_interface_INCLUDE_DIRS} ${rosidl_generator_cpp_INCLUDE_DIRS} ${rosidl_typesupport_opensplice_cpp_INCLUDE_DIRS} ${OPENSPLICE_INCLUDE_DIRS})
add_definitions(${OPENSPLICE_DEFINITIONS})
add_library(ros_middleware_opensplice_cpp SHARED src/functions.cpp)
target_link_libraries(ros_middleware_opensplice_cpp ${rosidl_typesupport_opensplice_cpp_LIBRARIES} ${OPENSPLICE_LIBRARIES})

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  TARGETS ros_middleware_opensplice_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
