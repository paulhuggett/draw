add_subdirectory(font)

add_library(draw STATIC )
target_sources(draw PRIVATE
  font.hpp
  bitmap.cpp bitmap.hpp
  glyph_cache.cpp glyph_cache.hpp
  plru_cache.hpp
  icubaby.hpp
  types.hpp
)
target_include_directories(draw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(draw PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(draw PUBLIC draw-font)
setup_target(draw)
