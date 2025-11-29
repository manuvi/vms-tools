/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */


#include <vector>
#include <filesystem>
#include <iostream>
#include <system_error>

#include <sha_from_dir/options.h>
#include <sha_from_dir/process.h>

int main(int argc, char* argv[])
{
  Options options;
  OptionsParser parser;
  if (!parser.parse(argc, argv, options))
  {
    return EXIT_FAILURE;
  }

  if (!std::filesystem::exists(options.scanDir.value()))
  {
    std::cerr << "Error! Path " << options.scanDir.value() << " doesn't exist!\n";
    return EXIT_FAILURE;
  }

  if (!std::filesystem::is_directory(options.scanDir.value()))
  {
    std::cerr << "Error! Path " << options.scanDir.value() << " is not a directory!\n";
    return EXIT_FAILURE;
  }

  std::vector<std::filesystem::path> dir_list;
  if ( options.singleDir )
  {
    dir_list.push_back(options.scanDir.value());
  }
  else
  {
    for (const auto& entry : std::filesystem::directory_iterator(options.scanDir.value()))
    {
      if (entry.is_directory())
      {
        dir_list.push_back(entry);
      }
    }
  }

  if ( dir_list.empty() )
  {
    std::cerr << "Error! No directories to process inside " << options.scanDir.value() << "\n";
    return EXIT_FAILURE;
  }

  std::filesystem::path logPath = options.logPath.has_value() 
      ? options.logPath.value() : options.scanDir.value();

  if (!std::filesystem::exists(logPath))
  {
    std::error_code ec;
    if (!std::filesystem::create_directories(logPath, ec))
    {
      std::cerr << "Error! Unable to create log directory at " << logPath << "\n";
      return EXIT_FAILURE;
    }
  }
  else if (!std::filesystem::is_directory(logPath))
  {
    std::cerr << "Error! Log path " << logPath << " is not a directory!\n";
    return EXIT_FAILURE;
  }

  DirProcessor processor;
  bool ok = true;
  for (const auto& tarPath : dir_list) 
  {
    ok &= processor.process(tarPath, logPath, options.sortEntries);
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
