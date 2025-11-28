/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#pragma once

#include <filesystem>

class TarProcessor
{
public:
  bool process(const std::filesystem::path& tarPath, const std::filesystem::path& logPath,
               bool sortEntries) const;
};
