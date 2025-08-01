add_executable(draw-unit-tests)
target_sources(draw-unit-tests PRIVATE
  test_copy.cpp
  test_frame_rect.cpp
  test_paint_rect.cpp
  test_line.cpp
  test_rect.cpp
)
target_link_libraries(draw-unit-tests PRIVATE gmock gtest gmock_main draw)
setup_target(draw-unit-tests)
