# If the compiler accepts the command-line flag given by the
# 'flag' parameter, 'result' is set to flag otherwise to an
# empty string.
function(flag_if_available flag result)
  string(MAKE_C_IDENTIFIER "DRAW_HAS_${flag}" flag_var)
  check_cxx_compiler_flag(${flag} ${flag_var})
  if (${flag_var})
    set(${result} ${flag})
  else()
    unset(${result})
  endif()
  return(PROPAGATE ${result})
endfunction(flag_if_available)

# setup target
# ~~~~~~~~~~~~
function (setup_target target)
  cmake_parse_arguments (
    arg # prefix
    "" # options
    "PEDANTIC" # one-value-keywords
    "" # multi-value-keywords
    ${ARGN}
  )
  # PEDANTIC defaults to "Yes".
  if ("${arg_PEDANTIC}" STREQUAL "")
    set (arg_PEDANTIC "Yes")
  endif ()

  target_compile_features(${target} PUBLIC cxx_std_23)

  set(clang_options
    -Weverything
    -Wno-c++14-extensions
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-exit-time-destructors
    -Wno-padded
    -Wno-weak-vtables

    # TODO: just temporary
    -Wno-global-constructors
  )
  set(gcc_options -Wall -Wextra -pedantic -Wno-maybe-uninitialized)
  set(msvc_options
    -W4     # enable lots of warnings
    -wd4068 # unknown pragma
    -wd4324 # structure was padded due to alignment specifier
  )
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    # Set flags that are not available in all versions of clang.
    set(extra_flags
      -Wno-c++20-compat
      -Wno-c++2a-compat
      -Wno-covered-switch-default
      -Wno-ctad-maybe-unsupported
      -Wno-nullability-extension
      -Wno-poison-system-directories
      -Wno-unsafe-buffer-usage
      -Wno-switch-enum
    )
    if (XCODE)
      # Xcode warns about several missing include directories inside the build tree. Just silence the noise.
      list(APPEND extra_flags -Wno-missing-include-dirs)
    endif()
    foreach(flag IN LISTS extra_flags)
      flag_if_available(${flag} new_flag)
      list(APPEND clang_options ${new_flag})
    endforeach()
  endif()

  target_compile_definitions(${target} PRIVATE $<$<BOOL:${DRAW_HOSTED}>:DRAW_HOSTED=1>)
  target_compile_options(${target}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
  )

  # werror
  target_compile_options(${target} PRIVATE
    $<$<AND:$<BOOL:${DRAW_WERROR}>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-Werror>
    $<$<AND:$<BOOL:${DRAW_WERROR}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
  )

  # coverage
  target_compile_options(${target} PRIVATE
    $<$<AND:$<BOOL:${DRAW_COVERAGE}>,$<CXX_COMPILER_ID:GNU>>:-fprofile-arcs -ftest-coverage>
    $<$<AND:$<BOOL:${DRAW_COVERAGE}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-fprofile-instr-generate -fcoverage-mapping>
  )
  target_link_options(${target} PRIVATE
    $<$<AND:$<BOOL:${DRAW_COVERAGE}>,$<CXX_COMPILER_ID:GNU>>:-fprofile-arcs -ftest-coverage>
    $<$<AND:$<BOOL:${DRAW_COVERAGE}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-fprofile-instr-generate -fcoverage-mapping>
  )

  set(ld64_link_options LINKER:-map,${CMAKE_CURRENT_BINARY_DIR}/${target}.map)
  set(ld_link_options LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${target}.map)
  set(msvc_link_options LINKER:/Map:${CMAKE_CURRENT_BINARY_DIR}/${target}.map)
  check_linker_flag(CXX "${ld64_link_options}" is_ld64)
  check_linker_flag(CXX "${ld_link_options}" is_ld)
  check_linker_flag(CXX "${msvc_link_options}" is_msvc)

  target_link_options(${target}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
    $<$<BOOL:${is_ld64}>:${ld64_link_options}>
    $<$<BOOL:${is_ld}>:${ld_link_options}>
    $<$<BOOL:${is_msvc}>:${msvc_link_options}>
  )
endfunction(setup_target)

