find_package(Curses)
if(Curses_FOUND AND NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )

  add_executable(draw-curses-count)
  target_sources(draw-curses-count PRIVATE curses_count.cpp)
  target_link_libraries(draw-curses-count PRIVATE draw)
  target_link_libraries(draw-curses-count PRIVATE Curses::Curses)
  setup_target(draw-curses-count)
endif()

add_executable(draw-time)
target_sources(draw-time PRIVATE time.cpp)
target_link_libraries(draw-time PRIVATE draw)
setup_target(draw-time)


add_executable(draw-all-chars)
target_sources(draw-all-chars PRIVATE all_chars.cpp)
target_link_libraries(draw-all-chars PRIVATE draw)
setup_target(draw-all-chars)
