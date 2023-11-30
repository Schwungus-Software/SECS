set(SOURCES "SECS.cpp")
set(HEADERS "SECS.hpp")

add_library(${PROJECT_TARGET} STATIC)

target_sources(
  ${PROJECT_TARGET}
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS})
