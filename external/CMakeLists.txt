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

set(qrcodegen "qrcodegen")
set(QR_Code_generator_srcs QR-Code-generator/cpp/qrcodegen.cpp) # or QR-Code-generator/c/qrcodegen.c
add_library(${qrcodegen} STATIC ${QR_Code_generator_srcs})
target_include_directories(${qrcodegen} PUBLIC QR-Code-generator/cpp/)

set(sqlite3 "sqlite3")
set(sqlite_srcs sqlite/sqlite3.c)
add_library(${sqlite3} STATIC ${sqlite_srcs})
target_include_directories(${sqlite3} PUBLIC sqlite/)
target_compile_definitions(${sqlite3} PUBLIC SQLITE_ENABLE_FTS5)

set(raft "raft")
set(raft_srcs raft/src/raft_server.c raft/src/raft_server_properties.c raft/src/raft_log.c raft/src/raft_node.c)
add_library(${raft} STATIC ${raft_srcs})
target_include_directories(${raft} PUBLIC raft/include/)

set(linenoise_target "linenoise")
set(linenoise_srcs linenoise/linenoise.c)
add_library(${linenoise_target} STATIC ${linenoise_srcs})
target_include_directories(${linenoise_target} PUBLIC linenoise/)

set(lua_target "lua")
set(lua_srcs lua/src/lapi.c lua/src/lcode.c lua/src/lctype.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c lua/src/lauxlib.c lua/src/lbaselib.c lua/src/lcorolib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loadlib.c lua/src/loslib.c lua/src/lstrlib.c lua/src/ltablib.c lua/src/lutf8lib.c lua/src/linit.c)
add_library(${lua_target} STATIC ${lua_srcs})
target_include_directories(${lua_target} PUBLIC lua/src/)

#[[
set(_target "")
set(_srcs )
add_library(${} STATIC ${_srcs})
target_include_directories(${} PUBLIC )
#set_target_properties(${} PROPERTIES PREFIX "")
]]
