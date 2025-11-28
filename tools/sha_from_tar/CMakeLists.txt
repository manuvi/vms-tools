find_package(LibArchive REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_console_tool(sha_from_tar
  SOURCES
    main.cpp
    options.cpp
    process.cpp
  DEPS
    LibArchive::LibArchive
    OpenSSL::Crypto
)
