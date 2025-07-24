add_library(draw STATIC 
  font.hpp
  bitmap.cpp bitmap.hpp
  glyph_cache.cpp glyph_cache.hpp
  icubaby.hpp
)
target_include_directories(draw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(draw PRIVATE cxx_std_23)
target_compile_options(draw PRIVATE 
  -Weverything
  -Wno-c++20-compat
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-exit-time-destructors
  -Wno-global-constructors
  -Wno-padded
  -Wno-poison-system-directories
)
