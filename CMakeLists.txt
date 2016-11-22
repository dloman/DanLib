CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(DanLib)
set(MODULE_NAME DanLib)
add_definitions(-DMODULE_NAME=\"DanLib\")

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-fPIC)

set(CMAKE_CXX_COMPILER /usr/local/bin/g++)

SET(DanLib_VERSION_MAJOR 0)
SET(DanLib_VERSION_MINOR 1)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX17)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
  ADD_DEFINITIONS(-std=c++1z)
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support.
    Please use a different C++ compiler.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Threads)

find_package(asio REQUIRED)

include_directories(${asio_INCLUDE_DIR})

#------------------------------------------------------------------------------
# Utility
#------------------------------------------------------------------------------

include_directories(Utility)

set(CMAKE_DEBUG_POSTFIX d)

add_library(
  String
  Utility/String/String.cpp
  Utility/String/String.hpp
  )

add_library(
  Signal
  Utility/Signal/Signal.hpp
  )

target_link_libraries(
  Signal
  ${CMAKE_THREAD_LIBS_INIT}
  )

set_target_properties(Signal PROPERTIES LINKER_LANGUAGE CXX)

add_executable(
  SignalTest
  Utility/Test/SignalTest.cpp
  )
target_link_libraries(
  SignalTest
  Signal
  )

add_executable(
  StringTest
  Utility/Test/StringTest.cpp
  )

target_link_libraries(
  StringTest
  String
  )

#------------------------------------------------------------------------------
# Tcp
#------------------------------------------------------------------------------

include_directories(Tcp)

add_library(
  Tcp
  Tcp/Tcp/Client.cpp
  Tcp/Tcp/Client.hpp
  Tcp/Tcp/Server.cpp
  Tcp/Tcp/Server.hpp
  Tcp/Tcp/Session.cpp
  Tcp/Tcp/Session.hpp
  )

target_link_libraries(
  Tcp
  Signal
  )

add_executable(
  Server
  Tcp/Test/ServerTest.cpp
  )

target_link_libraries(
  Server
  Tcp
  )

target_link_libraries(
  Tcp
  Signal
  )

add_executable(
  Client
  Tcp/Test/ClientTest.cpp
  )

target_link_libraries(
  Client
  Tcp
  )

add_executable(
  MultipleClientTest
  Tcp/Test/MultiClientTest.cpp
  )

target_link_libraries(
  MultipleClientTest
  Tcp
  )

add_executable(
  HttpServerTest
  Tcp/Test/HttpServer.cpp
  )

target_link_libraries(
  HttpServerTest
  String
  Tcp
  )

add_executable(
  HttpClientTest
  Tcp/Test/HttpClient.cpp
  )

target_link_libraries(
  HttpClientTest
  Tcp
  )

#------------------------------------------------------------------------------
# Serial
#------------------------------------------------------------------------------

include_directories(Serial)

add_library(
  Serial
  Serial/Serial/Serial.cpp
  Serial/Serial/Serial.hpp
  )

target_link_libraries(
  Serial
  Signal
  stdc++fs
  )

add_executable(
  SerialTest
  Serial/Test/SerialTest.cpp
  )

target_link_libraries(
  SerialTest
  Serial
  )

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------
set(DanLib_VERSION_STRING "${DanLib_VERSION_MAJOR}.${DanLib_VERSION_MINOR}")
set(DanLib_DIRNAME        "DanLib-${DanLib_VERSION_STRING}")

if (MSVC AND CMAKE_CL_64)
  set(DanLib_DIRNAME_lib "lib/${DanLib_DIRNAME}/x64")
elseif (MSVC)
  set(DanLib_DIRNAME_lib "lib/${DanLib_DIRNAME}/x86")
else ()
  set(DanLib_DIRNAME_lib "lib/${DanLib_DIRNAME}")
endif ()

set(DanLib_DIRNAME_include "include/${DanLib_DIRNAME}")

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DanLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DanLibConfig.cmake"
  INSTALL_DESTINATION
    ${DanLib_DIRNAME_lib}
  PATH_VARS
    DanLib_DIRNAME_lib
    DanLib_DIRNAME_include
  )

WRITE_BASIC_PACKAGE_VERSION_FILE(
  "${CMAKE_CURRENT_BINARY_DIR}/DanLibConfigVersion.cmake"
  VERSION ${DanLib_VERSION_STRING}
  COMPATIBILITY AnyNewerVersion
  )

install(
TARGETS
  Tcp
  Signal
  Serial
DESTINATION
  ${DanLib_DIRNAME_lib}
EXPORT
  DanLibExports
  )

install(
  EXPORT
    DanLibExports
  DESTINATION
    ${DanLib_DIRNAME_lib}
  )

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DanLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DanLibConfigVersion.cmake
  DESTINATION
    ${DanLib_DIRNAME_lib}
  )

install(
  FILES
    Utility/Signal/Signal.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Signal
  )

install(
  FILES
    Tcp/Tcp/Server.hpp
    Tcp/Tcp/Session.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Tcp
  )

install(
  FILES
    Serial/Serial/Serial.hpp
  DESTINATION
  ${DanLib_DIRNAME_include}/Serial
  )
