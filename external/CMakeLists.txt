cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(neroshop_external)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)

# compile the source code of external dependencies into a static library so there won't be a need to recompile each time there's a change in the makefile
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

if(DEFINED NEROSHOP_USE_LIBBCRYPT) # -DNEROSHOP_USE_LIBBCRYPT=1
set(bcrypt "bcrypt")
set(libbcrypt_srcs libbcrypt/crypt_blowfish/crypt_blowfish.c libbcrypt/crypt_blowfish/crypt_gensalt.c libbcrypt/crypt_blowfish/wrapper.c libbcrypt/bcrypt.c)
message(STATUS "Building libbcrypt: ${libbcrypt_srcs}")
add_library(${bcrypt} STATIC ${libbcrypt_srcs})
target_include_directories(${bcrypt} PUBLIC libbcrypt/)
endif()

set(qrcodegen "qrcodegen")
set(QR_Code_generator_srcs QR-Code-generator/cpp/qrcodegen.cpp) # or QR-Code-generator/c/qrcodegen.c
add_library(${qrcodegen} STATIC ${QR_Code_generator_srcs})
target_include_directories(${qrcodegen} PUBLIC QR-Code-generator/cpp/)

set(sqlite3 "sqlite3")
set(sqlite_srcs sqlite/sqlite3.c)
add_library(${sqlite3} STATIC ${sqlite_srcs})
target_include_directories(${sqlite3} PUBLIC sqlite/)

set(raft "raft")
set(raft_srcs raft/src/raft_log.c raft/src/raft_node.c raft/src/raft_server.c raft/src/raft_server_properties.c)
add_library(${raft} STATIC ${raft_srcs})
target_include_directories(${raft} PUBLIC raft/include/)

#[[
set(_target "")
set(_srcs )
add_library(${} STATIC ${_srcs})
target_include_directories(${} PUBLIC )
#set_target_properties(${} PROPERTIES PREFIX "")
]]
