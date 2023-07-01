cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

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
target_compile_definitions(${monero_cpp} PUBLIC -DBOOST_DISABLE_PRAGMA_MESSAGE)

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

set(i2pd_target "i2pd")
set(i2pd_srcs )
file(GLOB_RECURSE I2PD_CPP_FILES "i2pd/libi2pd/*.cpp")
foreach(CPP_FILE ${I2PD_CPP_FILES})
  list(APPEND i2pd_srcs ${CPP_FILE})
endforeach()
add_library(${i2pd_target} STATIC ${i2pd_srcs})
target_include_directories(${i2pd_target} PUBLIC i2pd/libi2pd/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations") # for deprecated OpenSSL functions

set(i2pd_client_target "i2pd_client")
set(i2pd_client_srcs )
file(GLOB_RECURSE I2PD_CLIENT_CPP_FILES "i2pd/libi2pd_client/*.cpp")
foreach(CPP_FILE ${I2PD_CLIENT_CPP_FILES})
  list(APPEND i2pd_client_srcs ${CPP_FILE})
endforeach()
add_library(${i2pd_client_target} STATIC ${i2pd_client_srcs})
target_include_directories(${i2pd_client_target} PUBLIC i2pd/i18n/ i2pd/libi2pd/ i2pd/libi2pd_client/)

add_subdirectory(miniupnp/miniupnpc)

add_subdirectory(libnatpmp)

add_subdirectory(libjuice)

#[[
set(_target "")
set(_srcs )
add_library(${} STATIC ${_srcs})
target_include_directories(${} PUBLIC )
#set_target_properties(${} PROPERTIES PREFIX "")
]]
