add_executable(unit_tests)
target_sources(unit_tests PRIVATE
  test_copy.cpp
  test_frame_rect.cpp
  test_paint_rect.cpp
  test_line.cpp
  test_rect.cpp
)
target_link_libraries(unit_tests PRIVATE gmock gtest gmock_main draw)
setup_target(unit_tests)
