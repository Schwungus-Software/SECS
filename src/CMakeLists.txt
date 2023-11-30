set(PROJECT_TARGET ${PROJECT_NAME})
set(TEST_TARGET ${PROJECT_NAME}_test)

add_library(${PROJECT_TARGET} "SECS.cpp" "SECS.hpp")

target_include_directories(
  ${PROJECT_TARGET} PUBLIC $<INSTALL_INTERFACE:{CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(SECS_BUILD_TEST)
  add_executable(${TEST_TARGET} "SECS_test.cpp")
  target_link_libraries(${TEST_TARGET} ${PROJECT_TARGET})
endif()
