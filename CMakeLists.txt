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

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

set(CMAKE_DEBUG_POSTFIX d)

#------------------------------------------------------------------------------
# Crypto
#------------------------------------------------------------------------------

include_directories(Crypto)


add_library(
  Crypto
  Crypto/Crypto/Sha.hpp
  )

set_target_properties(Crypto PROPERTIES LINKER_LANGUAGE CXX)

add_executable(
  CryptoTest
  Crypto/Tests/ShaTest.cpp
  )

target_link_libraries(
  CryptoTest
  Crypto
  )

#------------------------------------------------------------------------------
# Image
#------------------------------------------------------------------------------

include_directories(Image)

add_library(
  Image
  Image/Image/Image.cpp
  Image/Image/Image.hpp
  Image/Image/Bitmap.cpp
  Image/Image/Bitmap.hpp
  )

add_executable(
  BitmapTest
  Image/Tests/BitmapTest.cpp
  )

target_link_libraries(
  BitmapTest
  Image
  )

#------------------------------------------------------------------------------
# Utility
#------------------------------------------------------------------------------

include_directories(Utility)

add_library(
  Math
  Utility/Math/Vector.hpp
  )

set_target_properties(Math PROPERTIES LINKER_LANGUAGE CXX)

add_executable(
  MathTest
  Utility/Test/MathTest.cpp
  )

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
  Tcp/Tcp/Client.hpp
  Tcp/Tcp/Server.cpp
  Tcp/Tcp/Server.hpp
  Tcp/Tcp/Session.cpp
  Tcp/Tcp/Session.hpp
  Tcp/Tcp/SslSession.cpp
  Tcp/Tcp/SslSession.hpp
  Tcp/Tcp/SessionFactory.hpp
  )

target_link_libraries(
  Tcp
  Signal
  ${OPENSSL_LIBRARIES}
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
  SslClientTest
  Tcp/Test/SslClientTest.cpp
  )

target_link_libraries(
  SslClientTest
  Tcp
  )

#------------------------------------------------------------------------------
# Http
#------------------------------------------------------------------------------

include_directories(Http)

add_library(
  Http
  Http/Http/Responce.cpp
  Http/Http/Responce.hpp
  )

target_link_libraries(
  Http
  String
  )

add_executable(
  HttpServerTest
  Http/Utilities/HttpServer.cpp
  )

target_link_libraries(
  HttpServerTest
  String
  Tcp
  )

add_executable(
  HttpClientTest
  Http/Utilities/HttpClient.cpp
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
# Tcp
#------------------------------------------------------------------------------
include_directories(Requests)

add_library(
  Requests
  Requests/Requests/Requests.cpp
  Requests/Requests/Requests.hpp
  )

target_link_libraries(
  Requests
  Http
  Tcp
  )

add_executable(
  RequestsTest
  Requests/Test/RequestTest.cpp
  )

target_link_libraries(
  RequestsTest
  Requests
  Tcp
  )

#------------------------------------------------------------------------------
# Robot
#------------------------------------------------------------------------------

include_directories(Robot)

add_library(
  Robot
  Robot/Robot/PacketAssembler.cpp
  Robot/Robot/PacketAssembler.hpp
  Robot/Robot/PacketDecoder.hpp
  Robot/Robot/PacketEncoder.hpp
  Robot/Robot/Packets.hpp
  )

target_link_libraries(
  Robot
  Signal
  )

add_executable(
  RobotServer
  Robot/Utilities/RobotServer.cpp
  )

target_link_libraries(
  RobotServer
  Robot
  )

add_executable(
  RobotClient
  Robot/Utilities/RobotClient.cpp
  )

target_link_libraries(
  RobotClient
  Robot
  )

add_executable(
  RobotPacketTester
  Robot/Test/PacketTest.cpp
  )

target_link_libraries(
  RobotPacketTester
  Robot
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
  Http
  Image
  Requests
  Tcp
  Signal
  Serial
  String
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
    Http/Http/Responce.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Http
  )

install(
  FILES
    Image/Image/Image.hpp
    Image/Image/Bitmap.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Images
  )

install(
  FILES
    Requests/Requests/Requests.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Requests
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

install(
  FILES
    Utility/String/String.hpp
  DESTINATION
  ${DanLib_DIRNAME_include}/String
  )
