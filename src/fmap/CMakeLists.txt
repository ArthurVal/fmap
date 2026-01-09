add_library(${PROJECT_NAME}
  toto.c
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS fmap)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_features(${PROJECT_NAME}
  PUBLIC
  c_std_11
)

target_compile_options(${PROJECT_NAME}
  PUBLIC
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wdouble-promotion
  -Wundef
  -Wconversion
  # -Werror
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC $<$<STREQUAL:$<TARGET_PROPERTY:${PROJECT_NAME},TYPE>,SHARED_LIBRARY>:-D${PROJECT_PREFIX}_IS_SHARED>
  PRIVATE -D${PROJECT_PREFIX}_DO_EXPORT
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  # All symbols are NO_EXPORT by default
  C_VISIBILITY_PRESET hidden

  # Add the '-debug' when compiled in CMAKE_BUILD_TYPE=DEBUG
  DEBUG_POSTFIX "-debug"

  # Name used when exporting (otherwise will be the target name)
  EXPORT_NAME "${PROJECT_NAME}"

  # Version stuff for the export lib names, that handles symlinks
  # like:
  # libtoto.so -> libtoto.1.so -> libtoto.1.0.2.so
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPATIBLE_INTERFACE_STRING ${PROJECT_VERSION_MAJOR}
)

# install(
#   TARGETS ${PROJECT_NAME}
#   EXPORT ${PROJECT_NAME}Core
#   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# install(
#   EXPORT ${PROJECT_NAME}Core
#   NAMESPACE ${PROJECT_NAME}::
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
# )
