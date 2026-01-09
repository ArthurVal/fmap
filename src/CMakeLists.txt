add_subdirectory(fmap)

add_executable(fmap-bin fmap.c)

target_link_libraries(fmap-bin
  PRIVATE
  fmap::fmap
)
