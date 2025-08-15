add_executable(draw-unit-tests)
target_sources(draw-unit-tests PRIVATE
  test_copy.cpp
  test_frame_rect.cpp
  test_line.cpp
  test_paint_rect.cpp
  test_plru_cache.cpp
  test_rect.cpp
)
target_link_libraries(draw-unit-tests PRIVATE gmock gtest gmock_main draw)
setup_target(draw-unit-tests)

# Suppress some additional warnings.
set(clang_options -Wno-global-constructors -Wno-used-but-marked-unused)
set(gcc_options -Wno-pedantic)
set(msvc_options -wd4702) # unreachable code.

target_compile_options(draw-unit-tests
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
)
