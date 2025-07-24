add_executable(unit_tests)
target_compile_features(unit_tests PRIVATE cxx_std_23)
target_sources(unit_tests PRIVATE
  test_line.cpp
)
target_link_libraries(unit_tests PRIVATE gmock gtest gmock_main draw)