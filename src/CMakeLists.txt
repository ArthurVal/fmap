configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/fmap/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/_autogen/fmap/config.h
  @ONLY
)

add_executable(${PROJECT_NAME}
  fmap/logging.c
  fmap.c
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/_autogen>
)

target_compile_features(${PROJECT_NAME}
  PRIVATE
  c_std_11
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wdouble-promotion
  -Wundef
  -Wconversion
  # -Werror
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "fmap"

  # Add the '-debug' when compiled in CMAKE_BUILD_TYPE=DEBUG
  DEBUG_POSTFIX "-debug"

  VERSION ${PROJECT_VERSION}
  COMPATIBLE_INTERFACE_STRING ${PROJECT_VERSION_MAJOR}
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
