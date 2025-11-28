/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include "process.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

namespace
{
  struct HashedEntry
  {
    std::string name;
    std::string hash;
    std::uint64_t size = 0;
  };

  off_t get_file_size(const std::filesystem::path& filePath)
  {
    struct stat st
    {
    };
    if (stat(filePath.c_str(), &st) != 0)
    {
      return -1;
    }
    return st.st_size;
  }

  void print_progress(double progress)
  {
    constexpr int bar_width = 50;

    int pos = static_cast<int>(bar_width * progress / 100.0);

    std::cerr << "\r[";
    for (int i = 0; i < bar_width; ++i)
    {
      if (i < pos)
        std::cerr << '=';
      else if (i == pos)
        std::cerr << '>';
      else
        std::cerr << ' ';
    }
    std::cerr << "] ";

    std::cerr << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%" << std::flush;
  }
}  // namespace

bool TarProcessor::process(const std::filesystem::path& tarPath, const std::filesystem::path& logPath,
                           bool sortEntries) const
{
  std::filesystem::path logFileName = tarPath.stem().string() + ".sha256";  
  std::filesystem::path logFilePath = logPath / logFileName;
  
  archive* ar = archive_read_new();
  if (!ar)
  {
    std::cerr << "Unable to allocate libarchive reader\n";
    return false;
  }

  archive_read_support_filter_all(ar);
  archive_read_support_format_tar(ar);

  std::cout << "Processing file: " << tarPath << std::endl;

  if (archive_read_open_filename(ar, tarPath.c_str(), 10240) != ARCHIVE_OK)
  {
    std::cerr << "Unable to open file " << tarPath << ": " << archive_error_string(ar) << std::endl;
    archive_read_free(ar);
    return false;
  }

  off_t file_size = get_file_size(tarPath);

  std::vector<HashedEntry> entries;
  archive_entry* entry = nullptr;
  bool ok = true;
  la_int64_t last_bytes_read = 0;
  int log_sched = 0;

  while (true)
  {
    int headerRes = archive_read_next_header(ar, &entry);
    if (headerRes == ARCHIVE_EOF)
    {
      break;
    }
    if (headerRes != ARCHIVE_OK)
    {
      std::cerr << "Error reading header from " << tarPath << ": " << archive_error_string(ar) << std::endl;
      ok = false;
      break;
    }

    const char* nameC = archive_entry_pathname(entry);
    std::string name = nameC ? nameC : "";

    if (archive_entry_filetype(entry) != AE_IFREG)
    {
      continue;  // ignore directories and other types
    }

    std::uint64_t size = static_cast<std::uint64_t>(archive_entry_size(entry));

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> mdctx(EVP_MD_CTX_new(),
                                                                  &EVP_MD_CTX_free);
    if (!mdctx)
    {
      std::cerr << "Unable to allocate SHA256 context for " << name << std::endl;
      ok = false;
      break;
    }
    if (EVP_DigestInit_ex(mdctx.get(), EVP_sha256(), nullptr) != 1)
    {
      std::cerr << "Unable to initialize SHA256 for " << name << std::endl;
      ok = false;
      break;
    }

    while (true)
    {
      const void* buff = nullptr;
      size_t sizeBlock = 0;
      la_int64_t offset = 0;
      int dataRes = archive_read_data_block(ar, &buff, &sizeBlock, &offset);
      if (dataRes == ARCHIVE_EOF)
      {
        break;
      }
      if (dataRes != ARCHIVE_OK)
      {
        std::cerr << "Error reading data for " << name << ": " << archive_error_string(ar) << std::endl;
        ok = false;
        break;
      }
      la_int64_t current_bytes = archive_filter_bytes(ar, 0);

      if ( log_sched == 0 )
      {
        if (current_bytes != last_bytes_read) {
            double progress = (double)current_bytes / (double)file_size * 100.0;
            print_progress(progress);
            last_bytes_read = current_bytes;
        }
        log_sched++;
      }
      else
      {
        log_sched = (log_sched+1) % 10000;
      }

      if (sizeBlock > 0)
      {
        EVP_DigestUpdate(mdctx.get(), buff, sizeBlock);
      }
    }

    if (!ok)
    {
      break;
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> hash{};
    unsigned int hashLen = 0;
    if (EVP_DigestFinal_ex(mdctx.get(), hash.data(), &hashLen) != 1)
    {
      std::cerr << "Error finalizing SHA256 for " << name << std::endl;
      ok = false;
      break;
    }

    std::ostringstream hex;
    hex << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hashLen; ++i)
    {
      unsigned char b = hash[i];
      hex << std::setw(2) << static_cast<int>(b);
    }

    entries.push_back(HashedEntry{std::move(name), hex.str(), size});
  }

  print_progress(100.f);
  archive_read_close(ar);
  archive_read_free(ar);

  if (!ok)
  {
    return false;
  }

  if (sortEntries)
  {
    std::sort(entries.begin(), entries.end(),
              [](const HashedEntry& a, const HashedEntry& b) { return a.name < b.name; });
  }

  std::ofstream log(logFilePath);
  std::cout << std::endl << "Log file: " << logFilePath << std::endl;
  if (!log)
  {
    std::cerr << "Error! Cannot open " << logFilePath << " for writing" << std::endl;
    return false;
  }
  for (const auto& e : entries)
  {
    log << e.hash << "  " << e.name << std::endl;
  }

  return true;
}
