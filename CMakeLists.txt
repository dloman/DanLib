CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(DanLib)
set(MODULE_NAME DanLib)
add_definitions(-DMODULE_NAME=\"DanLib\")

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-fPIC)

set(CMAKE_CXX_COMPILER /usr/local/bin/g++)

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

include_directories(Tcp)

include_directories(Utility)

set(CMAKE_DEBUG_POSTFIX d)

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

add_library(
  TcpServer
  Tcp/Tcp/Server.cpp
  Tcp/Tcp/Server.hpp
  Tcp/Tcp/Session.cpp
  Tcp/Tcp/Session.hpp
  )

target_link_libraries(
  TcpServer
  Signal
  )

add_executable(
  Server
  Tcp/Test/ServerTest.cpp
  )

target_link_libraries(
  Server
  TcpServer
  )
