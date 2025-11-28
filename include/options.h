/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#pragma once

#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string_view>

struct Options
{
  std::filesystem::path searchDir = std::filesystem::current_path();
  std::optional<std::filesystem::path> archiveFile;
  std::optional<std::filesystem::path> logPath;
  bool sortEntries = false;
};

class OptionsParser
{
public:
  bool parse(int argc, char* argv[], Options& out) const;
  void print_usage(std::ostream& os) const;

private:

};
