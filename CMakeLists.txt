cmake_minimum_required(VERSION 4.0)
project(draw CXX)

find_package (Python3 COMPONENTS Interpreter)
if (NOT Python3_Interpreter_FOUND)
  message(FATAL_ERROR "Python3 interpreter not found") 
endif()

add_subdirectory(lib)

add_custom_target(buildfonts
  COMMAND
    "${Python3_EXECUTABLE}" font.py
    -o "${CMAKE_CURRENT_BINARY_DIR}"
    sans_font/sans.json
  DEPENDS font.py
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/sans.cpp"
             "${CMAKE_CURRENT_BINARY_DIR}/sans.hpp"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Building Sans Font"
  SOURCES
    font.py
    sans_font/sans.json
    sans_font/basic_latin-0000-000F.png
    sans_font/basic_latin-0010-001F.png
    sans_font/basic_latin-0020-002F.png
    sans_font/basic_latin-0030-003F.png
    sans_font/basic_latin-0040-004F.png
    sans_font/basic_latin-0050-005F.png
    sans_font/basic_latin-0060-006F.png
    sans_font/basic_latin-0070-007F.png
    sans_font/basic_latin-0080-008F.png
    sans_font/latin1-00A0-00AF.png
    sans_font/latin1-00B0-00BF.png
    sans_font/latin1-00C0-00CF.png
    sans_font/latin1-00D0-00DF.png
    sans_font/latin1-00E0-00EF.png
    sans_font/latin1-00F0-00FF.png
    sans_font/white-square-25A1.png
    sans_font/replacement-FFFD.png
)

add_executable(draw_demo)
add_dependencies(draw_demo buildfonts)
target_sources(draw_demo PRIVATE
  main.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/sans.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/sans.hpp"
)
target_include_directories(draw PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
)
target_compile_features(draw_demo PRIVATE cxx_std_23)
target_compile_options(draw_demo PRIVATE 
  -Weverything
  -Wno-c++20-compat
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-exit-time-destructors
  -Wno-global-constructors
  -Wno-padded
  -Wno-poison-system-directories
)
target_link_libraries(draw_demo PRIVATE draw)


# Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
# on Windows.
set(gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
# We don't want to install either gtest or gmock.
set(INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
set(INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/googletest")

add_subdirectory(unit_tests)
