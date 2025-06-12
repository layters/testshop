cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(neroshop_external)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build)

set(monero_include_dirs monero-cpp/external/monero-project/contrib/epee/include/ 
    monero-cpp/external/monero-project/external/easylogging++/ 
    monero-cpp/external/monero-project/src/ 
    monero-cpp/external/monero-project/external/rapidjson/include/ 
    monero-cpp/external/monero-project/external/ 
)
set(monero_cpp "monero-cpp")
set(monero_cpp_srcs monero-cpp/src/utils/gen_utils.cpp monero-cpp/src/utils/monero_utils.cpp monero-cpp/src/daemon/monero_daemon_model.cpp monero-cpp/src/daemon/monero_daemon.cpp monero-cpp/src/wallet/monero_wallet_model.cpp monero-cpp/src/wallet/monero_wallet_keys.cpp monero-cpp/src/wallet/monero_wallet_full.cpp)
add_library(${monero_cpp} STATIC ${monero_cpp_srcs})
target_include_directories(${monero_cpp} PUBLIC monero-cpp/src/ ${monero_include_dirs})
target_compile_definitions(${monero_cpp} PUBLIC BOOST_DISABLE_PRAGMA_MESSAGE)

set(qrcodegen "qrcodegen")
set(QR_Code_generator_srcs QR-Code-generator/cpp/qrcodegen.cpp) # or QR-Code-generator/c/qrcodegen.c
add_library(${qrcodegen} STATIC ${QR_Code_generator_srcs})
target_include_directories(${qrcodegen} PUBLIC QR-Code-generator/cpp/)

set(sqlite3 "sqlite3")
set(sqlite_srcs sqlite/sqlite3.c)
add_library(${sqlite3} STATIC ${sqlite_srcs})
target_include_directories(${sqlite3} PUBLIC sqlite/)
target_compile_definitions(${sqlite3} PUBLIC SQLITE_ENABLE_FTS5)
target_compile_definitions(${sqlite3} PUBLIC SQLITE_THREADSAFE=1) # 1=serialized mode (default), 0=single-thread, 2=multi-thread

set(linenoise_target "linenoise")
set(linenoise_srcs linenoise/linenoise.c)
add_library(${linenoise_target} STATIC ${linenoise_srcs})
target_include_directories(${linenoise_target} PUBLIC linenoise/)

set(lua_target "lua")
set(lua_srcs lua/src/lapi.c lua/src/lcode.c lua/src/lctype.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c lua/src/lauxlib.c lua/src/lbaselib.c lua/src/lcorolib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loadlib.c lua/src/loslib.c lua/src/lstrlib.c lua/src/ltablib.c lua/src/lutf8lib.c lua/src/linit.c)
add_library(${lua_target} STATIC ${lua_srcs})
target_include_directories(${lua_target} PUBLIC lua/src/)

set(zlib_target "zlib") # required by libi2pd
set(zlib_srcs zlib/adler32.c zlib/compress.c zlib/crc32.c zlib/deflate.c zlib/gzclose.c zlib/gzlib.c zlib/gzread.c zlib/gzwrite.c zlib/infback.c zlib/inffast.c zlib/inflate.c zlib/inftrees.c zlib/trees.c zlib/uncompr.c zlib/zutil.c)
add_library(${zlib_target} STATIC ${zlib_srcs})
target_include_directories(${zlib_target} PUBLIC zlib/)
set_target_properties(${zlib_target} PROPERTIES OUTPUT_NAME "z")
target_compile_options(${zlib_target} PUBLIC -Wno-implicit-function-declaration)

set(i2pd_target "i2pd")
set(i2pd_srcs )
file(GLOB_RECURSE I2PD_CPP_FILES "i2pd/libi2pd/*.cpp")
foreach(CPP_FILE ${I2PD_CPP_FILES})
  list(APPEND i2pd_srcs ${CPP_FILE})
endforeach()
add_library(${i2pd_target} STATIC ${i2pd_srcs})
target_include_directories(${i2pd_target} PUBLIC i2pd/libi2pd/)
add_definitions(-DOPENSSL_SUPPRESS_DEPRECATED)

set(i2pd_client_target "i2pdclient")
set(i2pd_client_srcs )
file(GLOB_RECURSE I2PD_CLIENT_CPP_FILES "i2pd/libi2pd_client/*.cpp")
foreach(CPP_FILE ${I2PD_CLIENT_CPP_FILES})
  list(APPEND i2pd_client_srcs ${CPP_FILE})
endforeach()
add_library(${i2pd_client_target} STATIC ${i2pd_client_srcs})
target_include_directories(${i2pd_client_target} PUBLIC i2pd/i18n/ i2pd/libi2pd/ i2pd/libi2pd_client/)

set(i2pd_lang_target "i2pdlang")
set(i2pd_lang_srcs )
file(GLOB_RECURSE I2PD_LANG_CPP_FILES "i2pd/i18n/*.cpp")
foreach(CPP_FILE ${I2PD_LANG_CPP_FILES})
  list(APPEND i2pd_lang_srcs ${CPP_FILE})
endforeach()
add_library(${i2pd_lang_target} STATIC ${i2pd_lang_srcs})
target_include_directories(${i2pd_lang_target} PUBLIC i2pd/i18n/ i2pd/libi2pd/ i2pd/libi2pd_client/)

set(fmt_target "fmt")
set(fmt_srcs fmt/src/format.cc fmt/src/os.cc)
add_library(${fmt_target} STATIC ${fmt_srcs})
target_include_directories(${fmt_target} PUBLIC fmt/include/)

find_library(SODIUM_LIBRARY sodium REQUIRED)
# Find libsodium include directory by locating sodium.h
find_path(SODIUM_INCLUDE_DIR sodium.h REQUIRED)
if(NOT SODIUM_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find libsodium include directory")
endif()
message(STATUS "Found libsodium include dir: ${SODIUM_INCLUDE_DIR}")
set(mkp224o_target "mkp224o")
file(GLOB ed25519_amd64_51_30k_srcs mkp224o/ed25519/amd64-51-30k/*.c)
file(GLOB ed25519_amd64_64_24k_srcs mkp224o/ed25519/amd64-64-24k/*.c)
file(GLOB ed25519_donna_srcs mkp224o/ed25519/ed25519-donna/*.c) # current default
file(GLOB ed25519_ref10_srcs mkp224o/ed25519/ref10/*.c)
set(mkp224o_srcs mkp224o/main.c mkp224o/worker.c mkp224o/yaml.c mkp224o/vec.c mkp224o/cpucount.c mkp224o/base32_to.c mkp224o/base32_from.c mkp224o/base64_to.c mkp224o/base64_from.c mkp224o/ioutil.c mkp224o/keccak.c)
#list(APPEND mkp224o_srcs ${ed25519_donna_srcs})
add_executable(${mkp224o_target} ${mkp224o_srcs})
set_target_properties(${mkp224o_target} PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_include_directories(${mkp224o_target} PUBLIC mkp224o/ mkp224o/ed25519/ mkp224o/ed25519/ed25519-donna/ ${SODIUM_INCLUDE_DIR})
target_compile_options(${mkp224o_target} PRIVATE #[[-std=c99]] -Wall -Wextra -Wno-maybe-uninitialized -Wno-unused-function -Wmissing-prototypes -Wstrict-prototypes -O3 -march=native -fomit-frame-pointer -fPIE)
find_package(Threads REQUIRED)
target_link_libraries(${mkp224o_target} PRIVATE Threads::Threads sodium) # -lpthread -lsodium

# cmake .
# make -j$(nproc)
