/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include "options.h"

#include <filesystem>
#include <iostream>
#include <string_view>
#include <sys/stat.h>

namespace fs = std::filesystem;

void OptionsParser::print_usage(std::ostream& os) const {
  os << "sha-from-tar — by Manuel Virgilio" << std::endl;
  os << "Compute the SHA-256 of every file contained in tar archives without extracting them." << std::endl;
  os << "Usage:" << std::endl;
  os << "  sha_from_tar [-f <archive> | -C <dir>] [-s] [-h]" << std::endl;
  os << "Options:" << std::endl;
  os << "  -f <archive> Specify a single .tar archive to scan\n";
  os << "  -C <dir>     Search for .tar archives in <dir> (defaults to current directory)" << std::endl;
  os << "  -O <dir>     Specify the directory to place sha files" << std::endl;
  os << "  -s           Sort file entries alphabetically within each archive" << std::endl;
  os << "  -h, --help   Show this help message" << std::endl;
}

bool OptionsParser::parse(int argc, char* argv[], Options& out) const {
  for (int i = 1; i < argc; ++i)
  {
    std::string_view arg{argv[i]};
    if (arg == "-h" || arg == "--help")
    {
      print_usage(std::cout);
      return false;
    }
    if (arg == "-f")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Error: -f requires a path" << std::endl;
        return false;
      }
      out.archiveFile = fs::path{argv[++i]};
      continue;
    }
    if (arg == "-C")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Error: -C requires a path" << std::endl;
        return false;
      }
      out.searchDir = fs::path{argv[++i]};
      continue;
    }
    if (arg == "-O")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Error: -O requires a path" << std::endl;
        return false;
      }
      out.logPath = fs::path(argv[++i]);
      continue;
    }
    if (arg == "-s")
    {
      out.sortEntries = true;
      continue;
    }

    std::cerr << "Unknown parameter: " << arg << std::endl;
    return false;
  }

  return true;
}
