/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "options.h"
#include "process.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
  Options options;
  OptionsParser parser;
  if (!parser.parse(argc, argv, options)) {
    return EXIT_FAILURE;
  }

  std::vector<fs::path> tarFiles;
  if (options.archiveFile) {
    if (!fs::exists(*options.archiveFile) || !fs::is_regular_file(*options.archiveFile)) {
      std::cerr << "Invalid file: " << *options.archiveFile << '\n';
      return EXIT_FAILURE;
    }
    tarFiles.push_back(*options.archiveFile);
  } else {
    if (!fs::exists(options.searchDir) || !fs::is_directory(options.searchDir)) {
      std::cerr << "Invalid directory: " << options.searchDir << '\n';
      return EXIT_FAILURE;
    }

    for (const auto& entry : fs::directory_iterator(options.searchDir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      if (entry.path().extension() == ".tar") {
        tarFiles.push_back(entry.path());
      }
    }
  }

  if (tarFiles.empty()) {
    std::cout << "No .tar archives found in " << options.searchDir << '\n';
    return EXIT_SUCCESS;
  }

  std::filesystem::path logPath = options.logPath.has_value() 
      ? options.logPath.value() : options.searchDir;

  TarProcessor processor;
  bool ok = true;
  for (const auto& tarPath : tarFiles) {
    ok &= processor.process(tarPath, logPath, options.sortEntries);
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
