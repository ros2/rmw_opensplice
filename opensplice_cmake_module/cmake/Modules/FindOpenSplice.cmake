###############################################################################
#
# CMake module for finding PrismTech OpenSplice.
#
# Input variables:
#
# - OSPL_HOME (optional): When specified, header files and libraries
#   will be searched for in `${OSPL_HOME}/include` and
#   `${OSPL_HOME}/lib` respectively.
#
# Output variables:
#
# - OpenSplice_FOUND: flag indicating if the package was found
# - OpenSplice_INCLUDE_DIRS: Paths to the header files
# - OpenSplice_LIBRARIES: Name to the C++ libraries including the path
# - OpenSplice_LIBRARY_DIRS: Paths to the libraries
# - OpenSplice_DEFINITIONS: Definitions to be passed on
# - OpenSplice_IDLPP: Path to the idl2code generator
#
# Example usage:
#
#   find_package(opensplice_cmake_module REQUIRED)
#   find_package(OpenSplice MODULE)
#   # use OpenSplice_* variables
#
###############################################################################

set(OpenSplice_FOUND FALSE)

# check if provided OSPL_HOME is from an "official" binary package
set(_ospl_home "$ENV{OSPL_HOME}")
if(WIN32)
  set(_ospl_release_file release.bat)
else()
  set(_ospl_release_file release.com)
endif()
if(NOT "${_ospl_home} " STREQUAL " " AND NOT EXISTS "${_ospl_home}/${_ospl_release_file}")
  set(_ospl_home "")
endif()
unset(_ospl_release_file)

if(NOT "${_ospl_home} " STREQUAL " ")
  # look inside of OSPL_HOME if defined
  message(STATUS "Found PrismTech OpenSplice: $ENV{OSPL_HOME}")
  set(OpenSplice_INCLUDE_DIRS
    "$ENV{OSPL_HOME}/include"
    "$ENV{OSPL_HOME}/include/sys"
    "$ENV{OSPL_HOME}/include/dcps/C++/SACPP")
  set(OpenSplice_LIBRARIES
    "cmxml"
    "commonserv"
    "dcpsgapi"
    "dcpssac"
    "ddsconfparser"
    "ddsconf"
    "ddsdatabase"
    "ddsi2"
    "ddskernel"
    "ddsosnet"
    "ddsos"
    "ddsserialization"
    "ddsuser"
    "ddsutil"
    "durability"
    "spliced"
    "dcpsisocpp"
    "dcpssacpp")
  list(APPEND OpenSplice_LIBRARIES "pthread" "dl")
  set(OpenSplice_LIBRARY_DIRS "$ENV{OSPL_HOME}/lib")
  set(OpenSplice_DEFINITIONS "")
  set(OpenSplice_IDLPP "$ENV{OSPL_HOME}/bin/idlpp")
  set(OpenSplice_FOUND TRUE)
else()
  # try to find_package() it
  find_package(opensplice NO_MODULE REQUIRED COMPONENTS CXX PATHS /usr /usr/local)
  if(OPENSPLICE_FOUND)
    message(STATUS "Found PrismTech OpenSplice: ${opensplice_DIR}")
    set(OpenSplice_HOME "${OPENSPLICE_PREFIX}")
    set(OpenSplice_INCLUDE_DIRS ${OPENSPLICE_INCLUDE_DIRS})
    set(OpenSplice_LIBRARIES ${OPENSPLICE_LIBRARIES})
    set(OpenSplice_LIBRARY_DIRS "")
    set(OpenSplice_DEFINITIONS ${OPENSPLICE_DEFINITIONS})
    set(OpenSplice_IDLPP "${OPENSPLICE_IDLPP}")
    set(OpenSplice_FOUND TRUE)
  endif()
endif()
