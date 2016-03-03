cmake_minimum_required(VERSION 2.8.3)

project(rmw_opensplice_cpp)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)

find_package(opensplice_cmake_module REQUIRED)
find_package(OpenSplice MODULE)
if(NOT OpenSplice_FOUND)
  ament_package()
  message(WARNING "Could not find PrismTech OpenSplice - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rosidl_typesupport_opensplice_c)
find_package(rosidl_typesupport_opensplice_cpp)
if(NOT rosidl_typesupport_opensplice_cpp_FOUND OR NOT rosidl_typesupport_opensplice_c_FOUND)
  ament_package()
  message(WARNING "Could not find ROSIDL TypeSupport for OpenSplice (rosidl_typesupport_opensplice_cpp or rosidl_typesupport_opensplice_c) - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rmw REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(introspection_msgs REQUIRED)

ament_export_dependencies(
  rmw
  rosidl_generator_cpp
  rosidl_typesupport_opensplice_c
  rosidl_typesupport_opensplice_cpp
)

link_directories(${OpenSplice_LIBRARY_DIRS})
add_library(rmw_opensplice_cpp SHARED
  src/identifier.cpp
  src/qos.cpp
  src/rmw_client.cpp
  src/rmw_compare_gids_equal.cpp
  src/rmw_count.cpp
  src/rmw_get_gid_for_publisher.cpp
  src/rmw_get_implementation_identifier.cpp
  src/rmw_guard_condition.cpp
  src/rmw_init.cpp
  src/rmw_node.cpp
  src/rmw_publish.cpp
  src/rmw_publisher.cpp
  src/rmw_request.cpp
  src/rmw_response.cpp
  src/rmw_service.cpp
  src/rmw_subscription.cpp
  src/rmw_take.cpp
  src/rmw_topic_names_and_types.cpp
  src/rmw_trigger_guard_condition.cpp
  src/rmw_wait.cpp
  src/rmw_waitset.cpp
  src/types.cpp
)
ament_target_dependencies(rmw_opensplice_cpp
  "introspection_msgs"
  "std_msgs"
  "rmw"
  "rosidl_generator_cpp"
  "rosidl_typesupport_opensplice_c"
  "rosidl_typesupport_opensplice_cpp"
  "OpenSplice")
# On Windows this adds the RMW_BUILDING_DLL definition.
# On Unix (GCC or Clang) it hides the symbols with -fvisibility=hidden.
configure_rmw_library(rmw_opensplice_cpp)
# Additionally, on Windows, add the ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_BUILDING_DLL definition.
if(WIN32)
  target_compile_definitions(rmw_opensplice_cpp
    PRIVATE "_CRT_NONSTDC_NO_DEPRECATE")
  target_compile_definitions(rmw_opensplice_cpp
    PRIVATE "ROSIDL_TYPESUPPORT_OPENSPLICE_CPP_BUILDING_DLL")
endif()

ament_export_libraries(rmw_opensplice_cpp)

# register_rmw_implementation("rosidl_typesupport_opensplice_c;rosidl_typesupport_opensplice_cpp")
register_rmw_implementation("rosidl_typesupport_opensplice_cpp")

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(CONFIG_EXTRAS "${PROJECT_NAME}-extras.cmake")

install(
  TARGETS rmw_opensplice_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
