cmake_minimum_required(VERSION 3.18.4)

project(vms_tools LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ConsoleTool)

option(VMS_TOOLS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_subdirectory(tools)
