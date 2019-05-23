CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#fixes wierd library shortname isssue
CMAKE_POLICY( SET CMP0060 NEW )

SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)

PROJECT(DanLib)
set(MODULE_NAME DanLib)
add_definitions(-DMODULE_NAME=\"DanLib\")

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-fPIC)


SET(DanLib_VERSION_MAJOR 0)
SET(DanLib_VERSION_MINOR 1)

ADD_DEFINITIONS(-std=c++1z)
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

#TODO move to boost asio vs standalone

find_package(asio REQUIRED)

include_directories(${asio_INCLUDE_DIR})

find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIR})

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

set(CMAKE_DEBUG_POSTFIX d)

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

add_executable(
  RandomTest
  Utility/Test/RandomTest.cpp
  )

#------------------------------------------------------------------------------
# Tcp
#------------------------------------------------------------------------------

include_directories(Tcp)

add_library(
  DanLibTcp
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
  DanLibTcp
  Signal
  ${OPENSSL_LIBRARIES}
  )

add_executable(
  Server
  Tcp/Test/ServerTest.cpp
  )

target_link_libraries(
  Server
  DanLibTcp
  )

add_executable(
  Client
  Tcp/Test/ClientTest.cpp
  )

target_link_libraries(
  Client
  DanLibTcp
  )

add_executable(
  MultipleClientTest
  Tcp/Test/MultiClientTest.cpp
  )

target_link_libraries(
  MultipleClientTest
  DanLibTcp
  )

add_executable(
  SslClientTest
  Tcp/Test/SslClientTest.cpp
  )

target_link_libraries(
  SslClientTest
  DanLibTcp
  )

#------------------------------------------------------------------------------
# Udp
#------------------------------------------------------------------------------

include_directories(Udp)

add_library(
  Udp
  Udp/Udp/Server.cpp
  Udp/Udp/Server.hpp
  Udp/Udp/Session.cpp
  Udp/Udp/Session.hpp
  )

add_executable(
  UdpEcho
  Udp/Test/UdpEchoServer.cpp
  )

target_link_libraries(
  UdpEcho
  Udp
  ${CMAKE_THREAD_LIBS_INIT}
  )

##------------------------------------------------------------------------------
## Http
##------------------------------------------------------------------------------
#
#include_directories(Http)
#
#add_library(
#Http
#Http/Http/Responce.cpp
#Http/Http/Responce.hpp
#Http/Http/Request.cpp
#Http/Http/Request.hpp
#Http/Http/Server.cpp
#Http/Http/Server.hpp
#)
#
#target_link_libraries(
#Http
#DanLibTcp
#String
#)
#
#add_executable(
#HttpServerTest
#Http/Utilities/HttpServer.cpp
#)
#
#target_link_libraries(
#HttpServerTest
#String
#Http
  #)

  #add_executable(
  #HttpClientTest
  #Http/Utilities/HttpClient.cpp
  #)
  #
  #target_link_libraries(
  #HttpClientTest
  #DanLibTcp
  #)
  #
#------------------------------------------------------------------------------
# Websockets
#------------------------------------------------------------------------------
include_directories(Websockets)
#
#add_library(
#DanLibWebsockets
#Websockets/Websockets/Server.cpp
#Websockets/Websockets/Session.cpp
##Websockets/Websockets/SslSession.cpp
#Websockets/Websockets/Client.cpp
#)
#
#target_link_libraries(
#DanLibWebsockets
#${Boost_SYSTEM_LIBRARY}
#${CMAKE_THREAD_LIBS_INIT}
#${OPENSSL_LIBRARIES}
#)
#
#add_executable(
#WebSocketServerTest
#Websockets/Test/ServerTest.cpp
#)
#
#target_link_libraries(
#WebSocketServerTest
#DanLibWebsockets
#)
#
#add_executable(
#WebSocketClientTest
#Websockets/Test/ClientTest.cpp
#)
#
#target_link_libraries(
#WebSocketClientTest
#DanLibWebsockets
#${Boost_SYSTEM_LIBRARY}
#${CMAKE_THREAD_LIBS_INIT}
#)

#add_executable(
#WebSocketMultiClientTest
#Websockets/Test/MultiClientTest.cpp
#)
#
#target_link_libraries(
#WebSocketMultiClientTest
#DanLibWebsockets
#${Boost_SYSTEM_LIBRARY}
#${CMAKE_THREAD_LIBS_INIT}
#)

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
# Requests
#------------------------------------------------------------------------------
#include_directories(Requests)

#add_library(
#Requests
#Requests/Requests/Requests.cpp
#Requests/Requests/Requests.hpp
#)

#target_link_libraries(
#Requests
##Http
#DanLibTcp
#)
#
#add_executable(
#RequestsTest
#Requests/Test/RequestTest.cpp
#)

#target_link_libraries(
#RequestsTest
#Requests
#DanLibTcp
#)

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

add_executable(
  exampleBeastHttpServer
  exampleBeasthttpServer/http_server_async.cpp
  )

target_link_libraries(
  exampleBeastHttpServer
  Boost::system
  Boost::filesystem
  ${CMAKE_THREAD_LIBS_INIT}
  )

#------------------------------------------------------------------------------
# Type Traits
#------------------------------------------------------------------------------
include_directories(TypeTraits)

add_executable(
  TypeTraits
  TypeTraits/Tests/TypeTraitsTest.cpp
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
#DanLibWebsockets
#Http
  Image
  #Requests
  DanLibTcp
  Signal
  Serial
  String
  Udp
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
    Utility/Random/Random.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Random
  )

install(
  FILES
    Robot/Robot/PacketEncoder.hpp
    Robot/Robot/PacketDecoder.hpp
    Robot/Robot/Packets.hpp
  DESTINATION
  ${DanLib_DIRNAME_include}/Robot
  )
install(
  FILES
    Tcp/Tcp/Server.hpp
    Tcp/Tcp/Session.hpp
    Tcp/Tcp/SslSession.hpp
    Tcp/Tcp/SessionFactory.hpp
    Tcp/Tcp/Client.hpp
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
  Udp/Udp/Server.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/Udp
  )

install(
  FILES
    Utility/String/String.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/String
  )

install(
  FILES
    TypeTraits/TypeTraits/TypeTraits.hpp
  DESTINATION
    ${DanLib_DIRNAME_include}/TypeTraits
  )
