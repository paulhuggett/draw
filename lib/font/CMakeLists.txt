add_custom_command(
  COMMAND
    mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/draw"
  COMMAND
    "${Python3_EXECUTABLE}" font.py -o "${CMAKE_CURRENT_BINARY_DIR}/draw" sans_font/16/sans16.json
  COMMAND
    "${Python3_EXECUTABLE}" font.py -o "${CMAKE_CURRENT_BINARY_DIR}/draw" sans_font/32/sans32.json
  OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.hpp"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Building Sans Font"
  DEPENDS
    font.py
    sans_font/16/sans16.json
    sans_font/16/basic_latin-0000-000F.png
    sans_font/16/basic_latin-0010-001F.png
    sans_font/16/basic_latin-0020-002F.png
    sans_font/16/basic_latin-0030-003F.png
    sans_font/16/basic_latin-0040-004F.png
    sans_font/16/basic_latin-0050-005F.png
    sans_font/16/basic_latin-0060-006F.png
    sans_font/16/basic_latin-0070-007F.png
    sans_font/16/basic_latin-0080-008F.png
    sans_font/16/latin1-00A0-00AF.png
    sans_font/16/latin1-00B0-00BF.png
    sans_font/16/latin1-00C0-00CF.png
    sans_font/16/latin1-00D0-00DF.png
    sans_font/16/latin1-00E0-00EF.png
    sans_font/16/latin1-00F0-00FF.png
    sans_font/16/white-square-25A1.png
    sans_font/16/replacement-FFFD.png
    sans_font/32/sans32.json
    sans_font/32/basic_latin-0020-002F.png
    sans_font/32/basic_latin-0030-003F.png
)
add_library(draw-font STATIC )
target_sources(draw-font PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans16.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/draw/sans32.hpp"
  font.py
  sans_font/16/sans16.json
  sans_font/32/sans32.json
)

target_include_directories(draw-font
  PUBLIC "${DRAW_PROJECT_ROOT}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
)
setup_target(draw-font)
