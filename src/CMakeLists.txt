set(PROJECT_TARGET ${PROJECT_NAME})
set(TEST_TARGET ${PROJECT_NAME}_test)

set(SOURCES "SECS.cpp")
set(HEADERS "SECS.hpp")

add_library(${PROJECT_TARGET} ${SOURCES} ${HEADERS})

target_include_directories(
  ${PROJECT_TARGET} PUBLIC $<INSTALL_INTERFACE:{CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(SECS_BUILD_TEST)
  add_executable(${TEST_TARGET} "test.cpp")
  target_link_libraries(${TEST_TARGET} ${PROJECT_TARGET})
endif()
