/*
 * Copyright (c) 2025 Manuel Virgilio
 *
 * Licensed under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <iomanip>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <sha_from_dir/process.h>

namespace
{
  struct HashedEntry
  {
    std::string name;
    std::string hash;
  };

  /*
  usare

  "\033[<n>A": sposta il cursore su di n righe.
  "\033[K": cancella da cursore a fine riga.
  "\r": ritorno a inizio riga.

  */

  constexpr int bar_width = 50;

  void print_progress(double percent)
  {
    int pos = static_cast<int>(bar_width * percent / 100.0);
    
    std::cerr << "\033[2K[";
    for (int i = 0; i < bar_width; ++i)
    {
        std::cerr << (i < pos ? '=' : (i == pos ? '>' : ' '));
    }
    std::cerr << "] ";
  }

  void print_file_status(uint32_t file_idx, uint32_t file_total, const std::filesystem::path& path)
  {
    double progress = file_idx > 0
        ? (100.0 * static_cast<double>(file_idx) / static_cast<double>(file_total))
        : 0.0;

    if ( file_idx > 1 )
    {
        std::cerr << "\033[2A";
    }

    // Line 1
    std::cerr << "\r\033[K-->Processing " << path.string() << std::endl;

    // Line 2
    std::cerr << "\r\033[K";
    print_progress(progress);
    std::cerr << " " << file_idx << "/" << file_total << std::endl;
  }

  void print_data_status(uint64_t bytes_read, uint64_t bytes_total, bool first_call)
  {
    double progress = bytes_read > 0
        ? (100.0 * static_cast<double>(bytes_read) / static_cast<double>(bytes_total))
        : 0.0;

    std::cerr << "\r";
    if ( first_call )
    {
        std::cerr << "\033[K";
    }
    print_progress(progress);
    std::cerr << " " << bytes_read << "/" << bytes_total << " bytes";
  }

  const size_t chunkSize = 4 * 1024 * 1024;
}  // namespace

bool DirProcessor::process(const std::filesystem::path& scanDir, const std::filesystem::path& logPath,
               bool sortEntries) const
{
    std::filesystem::path logFileName = scanDir.stem().string() + ".sha256";  
    std::filesystem::path logFilePath = logPath / logFileName;

    std::vector<std::filesystem::path> path_list;
    std::cout << "Scanning " << scanDir << "..." << std::flush;
    try 
    {
      for (const auto& entry : std::filesystem::recursive_directory_iterator(scanDir))
      {
          if (entry.is_regular_file())
          {
              path_list.push_back(entry.path());
          }
      }
    } 
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << std::endl << "Error: " << e.what() << std::endl;
        return false;
    }
    std::cout << "Ok" << std::endl << std::flush;
    
    std::vector<HashedEntry> entries;
    std::size_t file_count = 0;
    for ( const auto&path : path_list )
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            std::cerr << "Unable to open file: " << path << "\n";
            return false;
        }

        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> mdctx(EVP_MD_CTX_new(),
                                                                    &EVP_MD_CTX_free);
        if (!mdctx)
        {
            std::cerr << "Unable to allocate SHA256 context for " << path.filename() << std::endl;
            return false;
        }
        if (EVP_DigestInit_ex(mdctx.get(), EVP_sha256(), nullptr) != 1)
        {
            std::cerr << "Unable to initialize SHA256 for " << path.filename() << std::endl;
            return false;
        }

        std::vector<std::uint8_t> buffer(chunkSize);
        const std::streamsize chunk = static_cast<std::streamsize>(buffer.size());
        uint64_t file_size = std::filesystem::file_size(path);
        uint64_t file_read = 0;

        std::filesystem::path absolute_path = std::filesystem::absolute(scanDir);
        std::filesystem::path parent_path = absolute_path.has_parent_path() ? absolute_path.parent_path() : absolute_path;
        std::filesystem::path relative_path = std::filesystem::relative(path, parent_path);

        print_file_status(static_cast<uint32_t>(file_count + 1),
            static_cast<uint32_t>(path_list.size()),
            relative_path);

        bool first_data_run = true;
        while (true) 
        {
            file.read(reinterpret_cast<char*>(buffer.data()), chunk);
            std::streamsize bytes_read = file.gcount();
            file_read += static_cast<uint64_t>(bytes_read);

            if (bytes_read > 0)
            {
                if (EVP_DigestUpdate(mdctx.get(), buffer.data(), static_cast<size_t>(bytes_read)) != 1)
                {
                    std::cerr << "Error updating SHA256 for " << path.filename() << std::endl;
                    return false;
                }
            }

            if (bytes_read < chunk)
            {
                if (!file.eof() && file.fail()) {
                    std::cerr << "Error reading file: " << path << "\n";
                    return false;
                }
                break;
            }
        
            print_data_status(file_read, file_size, first_data_run);
            first_data_run = false;
        }

        print_data_status(file_size, file_size, false);
        ++file_count;

        std::array<unsigned char, EVP_MAX_MD_SIZE> hash{};
        unsigned int hashLen = 0;
        if (EVP_DigestFinal_ex(mdctx.get(), hash.data(), &hashLen) != 1)
        {
            std::cerr << "Error finalizing SHA256 for " << path.filename() << std::endl;
            return false;
        }

        std::ostringstream hex;
        hex << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < hashLen; ++i)
        {
            unsigned char b = hash[i];
            hex << std::setw(2) << static_cast<int>(b);
        }

        entries.push_back(HashedEntry{std::move(relative_path), hex.str()});
    }

    if (sortEntries)
    {
        std::cout << std::endl << "Sorting results..." << std::flush;
        std::sort(entries.begin(), entries.end(),
                [](const HashedEntry& a, const HashedEntry& b) { return a.name < b.name; });
        std::cout << "Ok" << std::endl << std::flush;
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
