find_package(Curses)
if(Curses_FOUND AND NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )
endif()

add_executable(draw_demo)
target_sources(draw_demo PRIVATE main.cpp)
target_link_libraries(draw_demo PRIVATE draw draw-font)
target_link_libraries(draw_demo PRIVATE Curses::Curses)
setup_target(draw_demo)
