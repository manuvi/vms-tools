find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_console_tool(sha_from_dir
  SOURCES
    main.cpp
    options.cpp
    process.cpp
  DEPS
    OpenSSL::Crypto
)
