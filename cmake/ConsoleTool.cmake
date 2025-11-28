include(CMakeParseArguments)

function(_vms_set_common_warnings target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /permissive- /EHsc /Zc:__cplusplus)
    if(VMS_TOOLS_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target_name} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
    )
    if(VMS_TOOLS_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE -Werror)
    endif()
  endif()
endfunction()

function(add_console_tool target_name)
  cmake_parse_arguments(CT "" "OUTPUT_NAME" "SOURCES;DEPS" ${ARGN})

  if(NOT CT_SOURCES)
    message(FATAL_ERROR "add_console_tool(${target_name}) requires SOURCES")
  endif()

  add_executable(${target_name} ${CT_SOURCES})
  target_compile_features(${target_name} PRIVATE cxx_std_20)
  target_include_directories(${target_name} PRIVATE "${PROJECT_SOURCE_DIR}/include")

  if(CT_DEPS)
    target_link_libraries(${target_name} PRIVATE ${CT_DEPS})
  endif()

  if(CT_OUTPUT_NAME)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "${CT_OUTPUT_NAME}")
  endif()

  set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )

  _vms_set_common_warnings(${target_name})
endfunction()
