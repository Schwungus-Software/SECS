cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(SECS CXX)

option(BUILD_TEST "Include the test application in the build if set to ON" ON)

set(PROJECT_TARGET ${PROJECT_NAME})
set(TEST_TARGET ${PROJECT_NAME}_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES "src/SECS.cpp")
set(HEADERS "src/SECS.hpp")

add_library(${PROJECT_TARGET} STATIC ${SOURCES} ${HEADERS})

if(BUILD_TEST)
  add_executable(${TEST_TARGET} "src/test.cpp")
  target_link_libraries(${TEST_TARGET} ${PROJECT_TARGET})
endif()
