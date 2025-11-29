# CMake Toolkit for C++ Console Utilities
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Quick build
```
cmake -S . -B build
cmake --build build
./build/bin/sha_from_tar --help
./build/bin/sha_from_dir --help
```

## Layout
- `CMakeLists.txt`: top-level configuration and C++ standards.
- `cmake/ConsoleTool.cmake`: `add_console_tool` helper with common warnings.
- `tools/`: each subfolder is a tool.
- `include/`: headers shared across tools.

## Available tools
- `sha_from_tar`: computes SHA-256 for regular files inside a `.tar` archive, prints a progress bar, and writes a `.sha256` log file (saved to *log-path* when set, otherwise to the search directory). Result entries can be *sorted* by filename
- `sha_from_dir`: computes SHA-256 for regular files inside a directory tree, shows a two-line progress (files and bytes), and writes a `.sha256` log file (saved to *log-path* when set, otherwise beside the directory). Result entries can be *sorted* by filename

## Notes
- `VMS_TOOLS_WARNINGS_AS_ERRORS=ON` treats compiler warnings as errors.
- Executables are placed in `build/bin/`.
