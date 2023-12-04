cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  SECS
  VERSION 0.1.2
  DESCRIPTION "A sequential ECS library"
  LANGUAGES CXX)

option(SECS_BUILD_TEST "Include the test application in the build?" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src SECS)
