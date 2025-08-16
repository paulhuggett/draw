add_subdirectory(font)

add_library(draw STATIC )
target_sources(draw PRIVATE
  "${DRAW_PROJECT_ROOT}/include/draw/font.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/bitmap.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/glyph_cache.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/plru_cache.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/icubaby.hpp"
  "${DRAW_PROJECT_ROOT}/include/draw/types.hpp"

  bitmap.cpp
  glyph_cache.cpp
)
target_include_directories(draw
  INTERFACE "${DRAW_PROJECT_ROOT}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  INTERFACE "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(draw PUBLIC draw-font)
setup_target(draw)
