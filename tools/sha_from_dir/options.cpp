/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include <sha_from_dir/options.h>

#include <filesystem>
#include <iostream>
#include <string_view>
#include <sys/stat.h>

namespace fs = std::filesystem;

void OptionsParser::print_usage(std::ostream& os) const
{
  os << "sha-from-dir — by Manuel Virgilio" << std::endl;
  os << "Compute SHA-256 for files in a directory or for each subdirectory within a container." << std::endl;
  os << "Usage:" << std::endl;
  os << "  sha_from_dir [-d] [-O <dir>] [-s] [-h] <path>" << std::endl;
  os << "Options:" << std::endl;
  os << "  -d            Treat <path> as a single directory (default: treat it as a container of directories)" << std::endl;
  os << "  -O <dir>      Directory where .sha256 logs are written (default: <path>)" << std::endl;
  os << "  -s            Sort entries alphabetically in each log" << std::endl;
  os << "  -h, --help    Show this help message" << std::endl;
}

bool OptionsParser::parse(int argc, char* argv[], Options& out) const
{
  for (int i = 1; i < argc; ++i)
  {
    std::string_view arg{argv[i]};

    if (arg == "-h" || arg == "--help")
    {
      print_usage(std::cout);
      return false;
    }

    if (arg == "-d")
    {
      out.singleDir = true;
      continue;
    }

    if (arg == "-O")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Error: -O requires a path" << std::endl;
        return false;
      }
      out.logPath = fs::path{argv[++i]};
      continue;
    }

    if (arg == "-s")
    {
      out.sortEntries = true;
      continue;
    }

    if (!arg.empty() && arg.front() == '-')
    {
      std::cerr << "Unknown parameter: " << arg << std::endl;
      return false;
    }

    if (out.scanDir)
    {
      std::cerr << "Error: multiple paths specified; only one <path> is allowed" << std::endl;
      return false;
    }

    out.scanDir = fs::path{arg};
  }

  if (!out.scanDir)
  {
    std::cerr << "Error: missing <path> to scan" << std::endl;
    return false;
  }

  return true;
}
