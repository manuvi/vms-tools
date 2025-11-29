/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include <filesystem>

class DirProcessor
{
public:
  bool process(const std::filesystem::path& scanDir, const std::filesystem::path& logPath,
               bool sortEntries) const;
};