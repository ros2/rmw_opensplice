cmake_minimum_required(VERSION 2.8.3)

project(opensplice_cmake_module NONE)

find_package(ament_cmake REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(OpenSplice MODULE)
if(OpenSplice_HOME)
  normalize_path(OpenSplice_HOME "${OpenSplice_HOME}")
  ament_environment_hooks(env_hook/opensplice.sh.in)
endif()

ament_package(
  CONFIG_EXTRAS "opensplice_cmake_module-extras.cmake"
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})
