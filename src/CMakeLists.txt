add_executable(${PROJECT_NAME}
  logging.c
  fmap.c
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
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
  # Add the '-debug' when compiled in CMAKE_BUILD_TYPE=DEBUG
  DEBUG_POSTFIX "-debug"
)
