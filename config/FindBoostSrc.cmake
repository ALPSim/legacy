# - FindBoostSrc
# Find Boost source tree
#
#
#

option(BUILD_BOOST_DATE_TIME "Build Boost Date Time" TRUE)
option(BUILD_BOOST_FILESYSTEM "Build Boost Filesystem" TRUE)
option(BUILD_BOOST_PROGRAM_OPTIONS "Build Boost Program Options" TRUE)
option(BUILD_BOOST_REGEX "Build Boost Regex" TRUE)
option(BUILD_BOOST_SERIALIZATION "Build Boost Serialization" TRUE)
option(BUILD_BOOST_SYSTEM "Build Boost System" TRUE)
option(BUILD_BOOST_PYTHON "Build Boost Python" TRUE)
option(BUILD_BOOST_THREAD "Build Boost Thread" TRUE)
option(BUILD_BOOST_TEST "Build Boost Test" TRUE)

mark_as_advanced(BUILD_BOOST_DATE_TIME)
mark_as_advanced(BUILD_BOOST_FILESYSTEM)
mark_as_advanced(BUILD_BOOST_PROGRAM_OPTIONS)
mark_as_advanced(BUILD_BOOST_REGEX)
mark_as_advanced(BUILD_BOOST_SERIALIZATION)
mark_as_advanced(BUILD_BOOST_SYSTEM)
mark_as_advanced(BUILD_BOOST_PYTHON)
mark_as_advanced(BUILD_BOOST_THREAD)
mark_as_advanced(BUILD_BOOST_TEST)

if (NOT Boost_ROOT_DIR)
  if(BOOST_ROOT)
    find_path(Boost_ROOT_DIR "libs/filesystem/src/path.cpp" PATHS ${BOOST_ROOT})
  else(BOOST_ROOT)
    set(DIR0
      ${PROJECT_SOURCE_DIR}/..
      $ENV{HOME} $ENV{HOME}/src $ENV{HOME}/ALPS/src /usr/local /usr/local/src
      "$ENV{HOMEDRIVE}/Program Files"
      "$ENV{HOMEDRIVE}$ENV{HOMEPATH}"
      "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/src"
      "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/ALPS/src")
    # set(DIR1 boost boostsrc boost_1_41_0 boost_1_40_0 boost_1_39_0 boost_1_38_0 boostsrc_1_38_0)
    set(DIR1 boost boostsrc boost_1_43_0 boost_1_42_0 boost_1_41_0)
    set(_boost_SEARCH_PATH "")
    foreach(D0 ${DIR0})
      foreach(D1 ${DIR1})
        set(_boost_SEARCH_PATH ${_boost_SEARCH_PATH} ${D0}/${D1})
      endforeach(D1)
    endforeach(D0)
    find_path(Boost_ROOT_DIR "libs/filesystem/src/path.cpp" ${_boost_SEARCH_PATH})
  endif(BOOST_ROOT)
endif (NOT Boost_ROOT_DIR)

if(Boost_ROOT_DIR)
  set(Boost_INCLUDE_DIR ${Boost_ROOT_DIR} CACHE PATH "Boost Include Directory" FORCE)
endif(Boost_ROOT_DIR)

# check Boost version (from Modules/FindBoost.cmake)
if(Boost_INCLUDE_DIR)
  set(_boost_VERSION 0)
  set(_boost_LIB_VERSION "")
  file(READ "${Boost_INCLUDE_DIR}/boost/version.hpp" _boost_VERSION_HPP_CONTENTS)
  string(REGEX REPLACE ".*#define BOOST_VERSION ([0-9]+).*" "\\1" _boost_VERSION "${_boost_VERSION_HPP_CONTENTS}")
  string(REGEX REPLACE ".*#define BOOST_LIB_VERSION \"([0-9_]+)\".*" "\\1" _boost_LIB_VERSION "${_boost_VERSION_HPP_CONTENTS}")
  set(Boost_LIB_VERSION ${_boost_LIB_VERSION} CACHE INTERNAL "The library version string for boost libraries")
  set(Boost_VERSION ${_boost_VERSION} CACHE INTERNAL "The version number for boost libraries")
  MATH(EXPR Boost_MAJOR_VERSION "${Boost_VERSION} / 100000")
  MATH(EXPR Boost_MINOR_VERSION "${Boost_VERSION} / 100 % 1000")
  MATH(EXPR Boost_SUBMINOR_VERSION "${Boost_VERSION} % 100")
endif(Boost_INCLUDE_DIR)

if(Boost_ROOT_DIR)
  message(STATUS "Found Boost Source: ${Boost_ROOT_DIR}")
  message(STATUS "Boost Version: ${Boost_MAJOR_VERSION}_${Boost_MINOR_VERSION}_${Boost_SUBMINOR_VERSION}")
  if(Boost_MAJOR_VERSION LESS 1 OR Boost_MINOR_VERSION LESS 41)
    message(FATAL_ERROR "Boost library is too old")
  endif(Boost_MAJOR_VERSION LESS 1 OR Boost_MINOR_VERSION LESS 41)
else(Boost_ROOT_DIR)
  message(FATAL_ERROR "Boost Source not Found")
endif(Boost_ROOT_DIR)

# Avoid auto link of Boost library
add_definitions(-DBOOST_ALL_NO_LIB=1)
if(BUILD_SHARED_LIBS)
  add_definitions(-DBOOST_ALL_DYN_LINK=1)
else(BUILD_SHARED_LIBS)
  add_definitions(-DBOOST_ALL_STATIC_LINK=1)
endif(BUILD_SHARED_LIBS)

mark_as_advanced(Boost_INCLUDE_DIR)
