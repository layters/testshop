cmake_minimum_required(VERSION 3.5)
project(neroshop) # tested on Ubuntu 20.04 LTS (focal) with gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

option(NEROSHOP_BUILD_TESTS "Build neroshop tests" OFF)
option(NEROSHOP_BUILD_CLI "Build neroshop CLI" OFF) # CLI is for testing only. This is a GUI-based ecommerce app so BUILD_CLI will be off by default
option(NEROSHOP_BUILD_GUI "Build neroshop GUI" ON)
include(CMakeDependentOption)
cmake_dependent_option(NEROSHOP_USE_QT "Build neroshop with Qt" ON "NEROSHOP_BUILD_GUI" OFF)
option(UUID_SYSTEM_GENERATOR "Enable operating system uuid generator" OFF)
option(UUID_TIME_GENERATOR "Enable experimental time-based uuid generator" OFF)
cmake_dependent_option(NEROSHOP_USE_SYSTEM_SOCKETS "Build neroshop with system sockets" ON "NOT NEROSHOP_USE_LIBZMQ" OFF)
cmake_dependent_option(NEROSHOP_USE_LIBZMQ "Build neroshop with LibZMQ" ON "NOT NEROSHOP_USE_SYSTEM_SOCKETS" OFF)
cmake_dependent_option(NEROSHOP_USE_PROTOBUF "Enable Protocol Buffers" ON "NOT NEROSHOP_USE_MSGPACK" OFF)
cmake_dependent_option(NEROSHOP_USE_MSGPACK "Enable MessagePack" ON "NOT NEROSHOP_USE_PROTOBUF" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/monero-project/cmake")
######################################
set(NEROSHOP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NEROSHOP_CONSOLE_SRC_DIR ${NEROSHOP_SRC_DIR}/console)
set(NEROSHOP_CORE_SRC_DIR ${NEROSHOP_SRC_DIR}/core)
set(NEROSHOP_DAEMON_SRC_DIR ${NEROSHOP_SRC_DIR}/daemon)
set(NEROSHOP_GUI_SRC_DIR ${NEROSHOP_SRC_DIR}/gui)

set(NEROSHOP_PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/proto)

######################################
if(NEROSHOP_BUILD_GUI AND NEROSHOP_USE_QT)
    set(CMAKE_AUTOMOC ON)
    #set(CMAKE_AUTORCC ON)
    #set(CMAKE_AUTOUIC ON)
    if(CMAKE_VERSION VERSION_LESS "3.7.0")
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()
    find_package(Qt5 COMPONENTS Qml Quick Gui Concurrent #[[QuickControls2]])# REQUIRED) # v5.15.x
    # add the qml.qrc file
    qt5_add_resources(neroshop_res qml.qrc)#qt_add_resources# <= introduced in Qt 5.15 and higher
endif()

######################################
if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;91m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif()

######################################
if(WIN32)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# Set compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -w")# -fsanitize=address") #-w=ignores all warnings
    message(STATUS "Building with clang")
else()
    message(STATUS "Building with gcc/g++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -no-pie")# -w")# -fsanitize=address") # -no-pie=can run outside terminal
endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif()

######################################
set(MONERO_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/monero-cpp/external/monero-project")
set(MONERO_SRC_DIR "${MONERO_ROOT_DIR}/src")

####################
# Extra libraries
if (APPLE)
  if (DEPENDS)
    list(APPEND EXTRA_LIBRARIES "-framework Foundation -framework IOKit -framework AppKit")
  else()
    find_library(COREFOUNDATION CoreFoundation)
    find_library(IOKIT IOKit)
    find_library(APPKIT AppKit)
    list(APPEND EXTRA_LIBRARIES ${IOKIT})
    list(APPEND EXTRA_LIBRARIES ${COREFOUNDATION})
    list(APPEND EXTRA_LIBRARIES ${APPKIT})
  endif()
endif()
if (WIN32)
  list(APPEND EXTRA_LIBRARIES setupapi)
endif()

message(STATUS EXTRA_LIBRARIES: ${EXTRA_LIBRARIES})

######################################
# Protobuf
include(FindProtobuf)
find_package(Protobuf)
message(STATUS "Protobuf lib: ${Protobuf_LIBRARY}, inc: ${Protobuf_INCLUDE_DIR}, protoc: ${Protobuf_PROTOC_EXECUTABLE}")

######################################
# LibUSB
find_library(usb_LIBRARY NAMES usb-1.0 libusb usb)
set(LibUSB_LIBRARIES ${usb_LIBRARY})

######################################
# Boost
set(Boost_NO_BOOST_CMAKE 1)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58 QUIET REQUIRED COMPONENTS chrono date_time filesystem program_options regex serialization wserialization system thread)
message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIR}")
add_definitions(-DBOOST_DISABLE_PRAGMA_MESSAGE)

######################################
# OpenSSL
if (APPLE AND NOT IOS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -std=c++14")
  if (NOT OPENSSL_ROOT_DIR)
      EXECUTE_PROCESS(COMMAND brew --prefix openssl
        OUTPUT_VARIABLE OPENSSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using OpenSSL found at ${OPENSSL_ROOT_DIR}")
  endif()
endif()

find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL include dir at ${OPENSSL_INCLUDE_DIR}")
add_definitions(-DOPENSSL_SUPPRESS_DEPRECATED)

if(STATIC AND NOT IOS)
  if(UNIX)
    set(OPENSSL_LIBRARIES "${OPENSSL_LIBRARIES};${CMAKE_DL_LIBS};${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()

######################################
# libsodium
find_library(SODIUM_LIBRARY sodium REQUIRED)
message(STATUS "Using libsodium: ${SODIUM_LIBRARY}")

######################################
# HIDAPI
if(APPLE)
  include_directories(SYSTEM /usr/include/malloc)
  if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
  endif()
endif()

find_package(HIDAPI)
if(HIDAPI_FOUND)
    message(STATUS "Using HIDAPI include dir at ${HIDAPI_INCLUDE_DIR}")
    add_definitions(-DHAVE_HIDAPI)
    link_directories(${LIBHIDAPI_LIBRARY_DIRS})
endif()
if(NOT HIDAPI_FOUND)
    message("HIDAPI libraries not found. Will manually set libraries.")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(UDEV_LIBRARY libudev.so)
        set(HIDAPI_LIBRARY ${UDEV_LIBRARY} libhidapi-libusb.so) # or libhidapi-hidraw.so # has to be dynamically linked or you'll get "undefined reference to `libusb_get_device_address'" error
    endif()
endif()

######################################
# Unbound
find_package(Unbound)
if ("${UNBOUND_LIBRARIES}" MATCHES "libunbound")
  message(STATUS "Using libunbound: ${UNBOUND_LIBRARIES}")
  add_library(unbound SHARED IMPORTED)
  set_target_properties(unbound PROPERTIES IMPORTED_LOCATION
      ${UNBOUND_LIBRARIES})
endif()

######################################
# Monero
set(MONERO_BUILD_DIR "${MONERO_ROOT_DIR}/build/release" CACHE STRING "Monero project build directory")
message(STATUS "Using monero-project build: " ${MONERO_BUILD_DIR})

add_library(wallet STATIC IMPORTED)
set_target_properties(wallet PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/lib/libwallet.a)
    
# libwallet-crypto.a provides x86_64 asm for some wallet functions
if (EXISTS ${MONERO_BUILD_DIR}/src/crypto/wallet/libwallet-crypto.a)
  add_library(wallet_crypto_lib STATIC IMPORTED)
  set_target_properties(wallet_crypto_lib PROPERTIES IMPORTED_LOCATION
      ${MONERO_BUILD_DIR}/src/crypto/wallet/libwallet-crypto.a)
  set(wallet_crypto wallet_crypto_lib)
endif()
    
add_library(lmdb STATIC IMPORTED)
set_target_properties(lmdb PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/external/db_drivers/liblmdb/liblmdb.a)
    
add_library(epee STATIC IMPORTED)
set_target_properties(epee PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/contrib/epee/src/libepee.a)
    
add_library(rpc_base STATIC IMPORTED)
set_target_properties(rpc_base PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/rpc/librpc_base.a)

add_library(net STATIC IMPORTED)
set_target_properties(net PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/net/libnet.a)
    
add_library(hardforks STATIC IMPORTED)
set_target_properties(hardforks PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/hardforks/libhardforks.a)    
    
add_library(easylogging STATIC IMPORTED)
set_target_properties(easylogging PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/external/easylogging++/libeasylogging.a)

add_library(cryptonote_core STATIC IMPORTED)
set_target_properties(cryptonote_core PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/cryptonote_core/libcryptonote_core.a)
    
    add_library(cryptonote_protocol STATIC IMPORTED)
set_target_properties(cryptonote_protocol PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/cryptonote_protocol/libcryptonote_protocol.a)

add_library(cryptonote_basic STATIC IMPORTED)
set_target_properties(cryptonote_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/cryptonote_basic/libcryptonote_basic.a)

add_library(cryptonote_format_utils_basic STATIC IMPORTED)
set_target_properties(cryptonote_format_utils_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/cryptonote_basic/libcryptonote_format_utils_basic.a)
    
add_library(mnemonics STATIC IMPORTED)
set_target_properties(mnemonics PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/mnemonics/libmnemonics.a)

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/common/libcommon.a)

add_library(cncrypto STATIC IMPORTED)
set_target_properties(cncrypto PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/crypto/libcncrypto.a)

add_library(ringct STATIC IMPORTED)
set_target_properties(ringct PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/ringct/libringct.a)

add_library(ringct_basic STATIC IMPORTED)
set_target_properties(ringct_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/ringct/libringct_basic.a)

add_library(blockchain_db STATIC IMPORTED)
set_target_properties(blockchain_db PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/blockchain_db/libblockchain_db.a)

add_library(blocks STATIC IMPORTED)
set_target_properties(blocks PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/blocks/libblocks.a)

add_library(checkpoints STATIC IMPORTED)
set_target_properties(checkpoints PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/checkpoints/libcheckpoints.a)

add_library(device STATIC IMPORTED)
set_target_properties(device PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/device/libdevice.a)
    
add_library(device_trezor STATIC IMPORTED)
set_target_properties(device_trezor PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/device_trezor/libdevice_trezor.a)

add_library(multisig STATIC IMPORTED)
set_target_properties(multisig PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/multisig/libmultisig.a)

add_library(version STATIC IMPORTED)
set_target_properties(version PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/src/libversion.a)
    
add_library(randomx STATIC IMPORTED)
set_target_properties(randomx PROPERTIES IMPORTED_LOCATION
    ${MONERO_BUILD_DIR}/external/randomx/librandomx.a)   
    
######################################
set(monero_include_dirs ${MONERO_ROOT_DIR}/contrib/epee/include
    ${MONERO_ROOT_DIR}/external/
    ${MONERO_ROOT_DIR}/external/easylogging++
    ${MONERO_ROOT_DIR}/external/rapidjson/include
    ${MONERO_SRC_DIR}/
    ${MONERO_SRC_DIR}/wallet
    ${MONERO_SRC_DIR}/wallet/api
    ${MONERO_SRC_DIR}/hardforks
    ${MONERO_SRC_DIR}/crypto
    ${MONERO_SRC_DIR}/crypto/crypto_ops_builder/include/
    ${Protobuf_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/libsodium/include/sodium
    #[[${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/openssl-sdk/include]]
    ${HIDAPI_INCLUDE_DIR}
    ${UNBOUND_INCLUDE_DIR}
)
set(monero_src wallet rpc_base net lmdb unbound easylogging cryptonote_core cryptonote_protocol cryptonote_basic cryptonote_format_utils_basic mnemonics ringct ringct_basic common blockchain_db blocks checkpoints device device_trezor cncrypto multisig version randomx epee hardforks ${wallet_crypto} 
${UNBOUND_LIBRARIES} ${Boost_LIBRARIES} ${Protobuf_LIBRARY} ${LibUSB_LIBRARIES} ${OPENSSL_LIBRARIES} ${SODIUM_LIBRARY} ${HIDAPI_LIBRARY} ${EXTRA_LIBRARIES})

######################################
# monero-cpp (wrapper)
set(monero_cpp_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/src/)
set(monero_cpp_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libmonero-cpp.a)

######################################
# sqlite3
set(sqlite_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite) # v3.39.1 (amalgamation - said to run a bit faster)
set(sqlite_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libsqlite3.a)

######################################
# QR-Code-generator
set(qrcodegen_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/QR-Code-generator/cpp) # v1.8.0
set(qrcodegen_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libqrcodegen.a)

######################################
# json (will soon be deprecated in favor of Monero-native rapidjson)
set(json_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include) # v3.12.0

######################################
# curl
find_package(CURL)
if(CURL_FOUND) 
    message(STATUS "Using CURL: ${CURL_LIBRARIES} (v${CURL_VERSION_STRING})")
    set(curl_include_dirs ${CURL_INCLUDE_DIRS})
    set(curl_src ${CURL_LIBRARIES})
endif()
if(NOT CURL_FOUND)
    set(curl_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib)
    set(curl_src ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib/.libs/libcurl.a) # In case user chooses to build libcurl themselves instead of installing it on the system
    add_definitions(-DCURL_STATICLIB)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lldap -llber -lnghttp2 -lpsl -lidn2 -lbrotlidec -lzstd -lrtmp") # for Arch (Manjaro)
endif()

######################################
# stduuid
set(stduuid_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/catch ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/include) # v1.2.3

######################################
# linenoise
set(linenoise_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/linenoise/)
set(linenoise_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblinenoise.a)

######################################
# lua
set(lua_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/lua/src/) # v5.4.4
set(lua_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblua.a)

######################################
# cxxopts
set(cxxopts_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts/include/) # v3.2.0

######################################
# zlib
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/libz.a") # v1.2.13
    set(zlib_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/)
    set(zlib_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libz.a)
    message(STATUS "Using ZLIB: ${zlib_src}")
else()
find_package(ZLIB)
if(ZLIB_FOUND)
    message(STATUS "Using ZLIB: ${ZLIB_LIBRARIES}")
    set(zlib_include_dir ${ZLIB_INCLUDE_DIRS})
    set(zlib_src ${ZLIB_LIBRARIES})
endif()
endif()

######################################
# libi2pd (and i2pd_client)
set(i2pd_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/external/i2pd/i18n ${CMAKE_CURRENT_SOURCE_DIR}/external/i2pd/libi2pd ${CMAKE_CURRENT_SOURCE_DIR}/external/i2pd/libi2pd_client)
set(i2pd_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libi2pdclient.a ${CMAKE_CURRENT_SOURCE_DIR}/build/libi2pd.a ${CMAKE_CURRENT_SOURCE_DIR}/build/libi2pdlang.a)

######################################
# fmt
set(fmt_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/fmt/include) # v11.1.4
set(fmt_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libfmt.a)

######################################
# mkp224o
set(mkp224o_include_dir ) # v1.7.0
set(mkp224o_src )

######################################
# msgpack-c
set(msgpack_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/msgpack-c/include) # v6.1.0
set(msgpack_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libmsgpack-c.a)

######################################
# protoc-generated .pb.cc files
if(EXISTS "${NEROSHOP_PROTO_SRC_DIR}/message.pb.cc" AND NOT NEROSHOP_USE_MSGPACK)
    set(neroshop_proto_src ${NEROSHOP_PROTO_SRC_DIR}/message.pb.cc)
    set(NEROSHOP_USE_PROTOBUF ON CACHE BOOL "Enable Protocol Buffers" FORCE)
    message(STATUS "Found .pb file(s): ${neroshop_proto_src}")
endif()

message(STATUS "${BoldYellow}NEROSHOP_USE_PROTOBUF: ${NEROSHOP_USE_PROTOBUF}${ColourReset}")
message(STATUS "${BoldYellow}NEROSHOP_USE_MSGPACK: ${NEROSHOP_USE_MSGPACK}${ColourReset}")

######################################
# neroshop link directories
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build) # target_link_directories only available in CMake version 3.13 :(

######################################
# neroshop core source files
set(neroshop_crypto_src 
    ${NEROSHOP_CORE_SRC_DIR}/crypto/rsa.cpp
    ${NEROSHOP_CORE_SRC_DIR}/crypto/sha256.cpp
    ${NEROSHOP_CORE_SRC_DIR}/crypto/sha3.cpp
)

set(neroshop_database_src 
    ${NEROSHOP_CORE_SRC_DIR}/database/database.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/database/sqlite3/sqlite3.cpp
)

set(neroshop_market_src 
    ${NEROSHOP_CORE_SRC_DIR}/market/cart.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/escrow.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/listing.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/order.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/product.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/seller.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/market/user.cpp 
)

set(neroshop_network_src 
    ${NEROSHOP_CORE_SRC_DIR}/network/onion_address.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/network/sam_client.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/network/socks5_client.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/network/tor_config.cpp
)

set(neroshop_price_src 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/coincodex/coincodex.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/coingecko/coingecko.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/coinmarketcap/coinmarketcap.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/cointelegraph/cointelegraph.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/cryptorank/cryptorank.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/cryptowatch/cryptowatch.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/fawazahmed0/fawazahmed0.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/kraken/kraken.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/price_api/currency_converter.cpp
)

set(neroshop_protocol_src 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/file_piece_hasher.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/key_mapper.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/node.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/routing_table.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/serializer.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/json_rpc.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/msgpack.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/protobuf.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/transport/client.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/protocol/transport/server.cpp
)

set(neroshop_tools_src 
    ${NEROSHOP_CORE_SRC_DIR}/tools/base64.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/tools/process.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/tools/script.cpp 
    ${NEROSHOP_CORE_SRC_DIR}/tools/timestamp.cpp 
)

set(neroshop_wallet_src 
    ${NEROSHOP_CORE_SRC_DIR}/wallet/wallet.cpp 
    #[[${NEROSHOP_CORE_SRC_DIR}/wallet/monero/monero.cpp]]
)

set(neroshop_client_src ${neroshop_crypto_src} ${neroshop_database_src} ${neroshop_market_src} #[[${neroshop_network_src}]] ${neroshop_price_src} ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/file_piece_hasher.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/serializer.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/json_rpc.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/transport/client.cpp ${neroshop_tools_src} ${neroshop_wallet_src} ${NEROSHOP_CORE_SRC_DIR}/settings.cpp)

######################################
# neroshop tests
if(NEROSHOP_BUILD_TESTS) # -DNEROSHOP_BUILD_TESTS=1
    enable_testing()
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_TESTS option set to ON${ColourReset}")
    add_subdirectory(tests)
endif()

######################################
# neroshopd
set(neroshop_daemon "neroshopd")
set(neroshop_daemon_src ${neroshop_proto_src} ${neroshop_crypto_src} ${neroshop_database_src} ${neroshop_network_src} ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/file_piece_hasher.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/key_mapper.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/node.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/p2p/routing_table.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/json_rpc.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/msgpack.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/rpc/protobuf.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/transport/client.cpp ${NEROSHOP_CORE_SRC_DIR}/protocol/transport/server.cpp ${NEROSHOP_CORE_SRC_DIR}/tools/base64.cpp ${NEROSHOP_CORE_SRC_DIR}/tools/timestamp.cpp)
add_executable(${neroshop_daemon} src/daemon/main.cpp ${neroshop_daemon_src})
install(TARGETS ${neroshop_daemon} DESTINATION bin)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_definitions(${neroshop_daemon} PRIVATE NEROSHOP_DEBUG)
endif()
target_include_directories(${neroshop_daemon} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]] ${monero_include_dirs} ${monero_cpp_include_dir} ${sqlite_include_dir} ${json_include_dir} ${msgpack_include_dir} ${zlib_include_dir} #[[${i2pd_include_dirs}]] ${fmt_include_dir} ${cxxopts_include_dir})
target_link_libraries(${neroshop_daemon} #[[${i2pd_src}]] ${monero_cpp_src} ${sqlite_src} ${msgpack_src} ${monero_src} #[[${zlib_src}]] ${fmt_src})#set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl") # fixes undefined reference to symbol 'dlsym@@GLIBC_2.2.5' error
if(NEROSHOP_USE_PROTOBUF)
    target_compile_definitions(${neroshop_daemon} PRIVATE NEROSHOP_USE_PROTOBUF)
endif()

######################################
# neroshop-console
if(NEROSHOP_BUILD_CLI)
    set(neroshop_cli "neroshop-console")
    add_executable(${neroshop_cli} ${neroshop_proto_src} src/console/main.cpp ${neroshop_client_src})
    install(TARGETS ${neroshop_cli} DESTINATION bin)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_definitions(${neroshop_cli} PRIVATE NEROSHOP_DEBUG)
    endif()
    target_include_directories(${neroshop_cli} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]] ${monero_include_dirs} ${monero_cpp_include_dir} ${sqlite_include_dir} ${json_include_dir} ${msgpack_include_dir} ${zlib_include_dir} #[[${i2pd_include_dirs}]] ${fmt_include_dir} ${lua_include_dir} ${curl_include_dirs} ${stduuid_include_dirs} ${linenoise_include_dir})
    target_link_libraries(${neroshop_cli} #[[${i2pd_src}]] ${monero_cpp_src} ${sqlite_src} ${msgpack_src} #[[${qrcodegen_src}]] ${curl_src} ${monero_src} #[[${zlib_src}]] ${fmt_src} ${lua_src} ${linenoise_src})
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_CLI option set to ON${ColourReset}")
    target_compile_definitions(${neroshop_cli} PRIVATE NEROSHOP_BUILD_CLI)
    if(NEROSHOP_USE_PROTOBUF)
        target_compile_definitions(${neroshop_cli} PRIVATE NEROSHOP_USE_PROTOBUF)
    endif()
endif()

######################################
# neroshop
if(NEROSHOP_BUILD_GUI)
    set(neroshop_gui "neroshop")
    add_executable(${neroshop_gui})
    install(TARGETS ${neroshop_gui} DESTINATION bin)
    target_sources(${neroshop_gui} PRIVATE ${neroshop_res} ${neroshop_proto_src} src/gui/main.cpp src/gui/settings_manager.cpp src/gui/wallet_manager.cpp src/gui/user_manager.cpp src/gui/image_loader.cpp src/gui/image_provider.cpp src/gui/wallet_qr_provider.cpp src/gui/currency_rate_provider.cpp src/gui/daemon_manager.cpp src/gui/enum_wrapper.cpp src/gui/proxy_manager.cpp src/gui/notification_manager.cpp src/gui/wallet_node_provider.cpp src/gui/backend.cpp ${neroshop_client_src})
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_DEBUG)
    endif()
    target_include_directories(${neroshop_gui} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]] ${monero_include_dirs} ${monero_cpp_include_dir} ${sqlite_include_dir} ${json_include_dir} ${msgpack_include_dir} ${zlib_include_dir} #[[${i2pd_include_dirs}]] ${fmt_include_dir} ${lua_include_dir} ${qrcodegen_include_dir})
    target_link_libraries(${neroshop_gui} #[[${i2pd_src}]] ${monero_cpp_src} ${sqlite_src} ${msgpack_src} ${qrcodegen_src} ${monero_src} #[[${zlib_src}]] ${fmt_src} ${lua_src})
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_GUI option set to ON${ColourReset}")
    target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_BUILD_GUI)
    if(NEROSHOP_USE_PROTOBUF)
        target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_USE_PROTOBUF)
    endif()
endif()

######################################
add_subdirectory(assets)

######################################
# system sockets
if(NOT NEROSHOP_USE_LIBZMQ)
    message(STATUS "${BoldWhite}Using System Sockets${ColourReset}")
    if(NEROSHOP_BUILD_GUI)
        target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_USE_SYSTEM_SOCKETS)
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_compile_definitions(${neroshop_cli} PRIVATE NEROSHOP_USE_SYSTEM_SOCKETS)
    endif()
    target_compile_definitions(${neroshop_daemon} PRIVATE NEROSHOP_USE_SYSTEM_SOCKETS)
endif()

######################################
# libzmq
find_package(ZeroMQ)
if(ZeroMQ_FOUND)
    if(NEROSHOP_USE_LIBZMQ)
        message(STATUS "${BoldRed}Using ZeroMQ: ${ZeroMQ_LIBRARY} (v${ZeroMQ_VERSION})${ColourReset}")
        if(NEROSHOP_BUILD_GUI)
            target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_USE_LIBZMQ)
            target_link_libraries(${neroshop_gui} ${ZeroMQ_LIBRARIES})
        endif()
        if(NEROSHOP_BUILD_CLI)
            target_compile_definitions(${neroshop_cli} PRIVATE NEROSHOP_USE_LIBZMQ)
            target_link_libraries(${neroshop_cli} ${ZeroMQ_LIBRARIES})
        endif()
        target_compile_definitions(${neroshop_daemon} PRIVATE NEROSHOP_USE_LIBZMQ)
        target_link_libraries(${neroshop_daemon} ${ZeroMQ_LIBRARIES})
    endif()
endif()

######################################
if (UUID_TIME_GENERATOR) # -DUUID_TIME_GENERATOR=1
    if(NEROSHOP_BUILD_CLI)
        target_compile_definitions(${neroshop_cli} PRIVATE UUID_TIME_GENERATOR)
    endif()    
endif()

######################################
if(WIN32)
    set(winsock2_src ws2_32.lib) # -lglfw3 -lgdi32 -lopengl32 -limm32
    if(NEROSHOP_BUILD_GUI)
        target_link_libraries(${neroshop_gui} ${winsock2_src})
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_link_libraries(${neroshop_cli} ${winsock2_src})
    endif()    
    target_link_libraries(${neroshop_daemon} ${winsock2_src})
endif()

######################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(LibUUID) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        if (LIBUUID_FOUND)
            message(STATUS "Using LibUUID: ${LibUUID_LIBRARY}")
            if(NEROSHOP_BUILD_CLI)
                target_compile_definitions(${neroshop_cli} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
                target_link_libraries(${neroshop_cli} ${LibUUID_LIBRARY})        
            endif()
        endif()
    endif()

    set(posix_src libpthread.so libdl.so libm.so)
    if(NEROSHOP_BUILD_GUI)
        target_link_libraries(${neroshop_gui} ${posix_src})
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_link_libraries(${neroshop_cli} ${posix_src})
    endif()
    target_link_libraries(${neroshop_daemon} ${posix_src})
endif()

######################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(CFLIB CoreFoundation) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        message(STATUS "Using CFLIB: ${CFLIB}")
        if(NEROSHOP_BUILD_CLI)
            target_compile_definitions(${neroshop_cli} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
            target_link_libraries(${neroshop_cli} ${CFLIB})
        endif()
    endif()
endif()

######################################
# qt5
if(Qt5_FOUND)
    if(NEROSHOP_BUILD_GUI AND NEROSHOP_USE_QT)
        # Some Linux distros require Qt::Widgets due to the lack of a native platform file dialog implementation
        if(NEROSHOP_USE_QT_WIDGETS OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            message(STATUS "Using Qt5::Widgets")
            target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_USE_QT_WIDGETS)
            find_package(Qt5 COMPONENTS Widgets)
            target_link_libraries(${neroshop_gui} Qt5::Widgets)
        endif()
    
        message(STATUS "Using Qt5::Qml Qt5::Quick Qt5::Gui Qt5::Concurrent")
        target_compile_definitions(${neroshop_gui} PRIVATE NEROSHOP_USE_QT)
        target_link_libraries(${neroshop_gui} Qt5::Qml Qt5::Quick Qt5::Gui Qt5::Concurrent #[[Qt5::QuickControls2]])
    endif()
endif()

######################################
