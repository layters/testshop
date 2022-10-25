cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(test)

set(neroshop_root ..)
set(neroshop_srcs ${neroshop_root}/src/core/buyer.cpp #[[${neroshop_root}/src/core/carrier.cpp]] ${neroshop_root}/src/core/cart.cpp ${neroshop_root}/src/core/catalog.cpp ${neroshop_root}/src/core/client.cpp ${neroshop_root}/src/core/converter.cpp ${neroshop_root}/src/core/database.cpp ${neroshop_root}/src/core/encryptor.cpp #[[${neroshop_root}/src/core/icon.cpp]] ${neroshop_root}/src/core/item.cpp #[[${neroshop_root}/src/core/message.cpp]] ${neroshop_root}/src/core/order.cpp  ${neroshop_root}/src/core/process.cpp ${neroshop_root}/src/core/qr.cpp ${neroshop_root}/src/core/script.cpp ${neroshop_root}/src/core/seller.cpp ${neroshop_root}/src/core/server.cpp ${neroshop_root}/src/core/user.cpp ${neroshop_root}/src/core/validator.cpp ${neroshop_root}/src/core/wallet.cpp)
#set(neroshop_headers ) # not needed
set(neroshop_include_dir ${neroshop_root}/include ${neroshop_root}/src)
include_directories(${neroshop_include_dir})
link_directories(${neroshop_root}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..) # will output to build/ instead of build/test/
######################################
if(WIN32)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# Set compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fPIE -fPIC") #-w=ignores all warnings # Qt requires -fPIC
    message(STATUS "Building with clang")
else()
    message(STATUS "Building with gcc/g++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie -fPIC") # detach # Qt requires -fPIC
endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif()
######################################
set(MONERO_PROJECT "${neroshop_root}/external/monero-cpp/external/monero-project")
set(MONERO_PROJECT_SRC "${MONERO_PROJECT}/src")

# header includes
include_directories("${MONERO_PROJECT}/contrib/epee/include")
include_directories("${MONERO_PROJECT}/external/")
include_directories("${MONERO_PROJECT}/external/easylogging++")
include_directories("${MONERO_PROJECT}/external/rapidjson/include")
include_directories("${MONERO_PROJECT_SRC}/")
include_directories("${MONERO_PROJECT_SRC}/wallet")
include_directories("${MONERO_PROJECT_SRC}/wallet/api")
include_directories("${MONERO_PROJECT_SRC}/crypto/crypto_ops_builder/include/")

set(EXTERNAL_LIBS_DIR ${neroshop_root}/monero-cpp/external-libs)
message(STATUS EXTERNAL_LIBS_DIR : ${EXTERNAL_LIBS_DIR})
######################################
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
include_directories(${Protobuf_INCLUDE_DIR})

######################################
# LibUSB
find_library(usb_LIBRARY NAMES usb-1.0 libusb usb)
set(LibUSB_LIBRARIES ${usb_LIBRARY})

######################################
# Boost - sudo apt install libboost-dev, sudo apt-get install libboost-all-dev
set(Boost_NO_BOOST_CMAKE 1)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58 QUIET REQUIRED COMPONENTS chrono date_time filesystem program_options regex serialization wserialization system thread)
message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIR}")
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) # to ignore warnings
######################################
# OpenSSL - sudo apt-get install libssl-dev
if (APPLE AND NOT IOS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -fvisibility=default -std=c++11")
  if (NOT OPENSSL_ROOT_DIR)
      EXECUTE_PROCESS(COMMAND brew --prefix openssl
        OUTPUT_VARIABLE OPENSSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using OpenSSL found at ${OPENSSL_ROOT_DIR}")
  endif()
endif()

find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL include dir at ${OPENSSL_INCLUDE_DIR}")
include_directories(${OPENSSL_INCLUDE_DIR})

if(STATIC AND NOT IOS)
  if(UNIX)
    set(OPENSSL_LIBRARIES "${OPENSSL_LIBRARIES};${CMAKE_DL_LIBS};${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()
######################################
# libsodium
include_directories(external/monero-cpp/external/libsodium/include)
include_directories(external/monero-cpp/external/libsodium/include/sodium)
find_library(SODIUM_LIBRARY sodium REQUIRED)
message(STATUS "Using libsodium library at ${SODIUM_LIBRARY}")
#[[include_directories(monero-cpp/external/libsodium/include/sodium) # export.h
set(SODIUM_LIBRARIES monero-cpp/external/libsodium/src/crypto_verify/verify.c)]]
#set(sodium_src /usr/lib/x86_64-linux-gnu/libsodium.so)
######################################
# HIDAPI - https://github.com/libusb/hidapi/releases / sudo apt-get install libhidapi-dev / sudo apt-get install libusb-1.0-0-dev, sudo apt-get install libudev-dev (libudev.h)
if(APPLE)
  include_directories(SYSTEM /usr/include/malloc)
  if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
  endif()
endif()

if (APPLE AND NOT IOS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -fvisibility=default -std=c++11")
  if (NOT OPENSSL_ROOT_DIR)
      EXECUTE_PROCESS(COMMAND brew --prefix openssl
        OUTPUT_VARIABLE OPENSSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using OpenSSL found at ${OPENSSL_ROOT_DIR}")
  endif()
endif()

find_package(HIDAPI)
if(HIDAPI_FOUND)
    message(STATUS "Using HIDAPI include dir at ${HIDAPI_INCLUDE_DIR}")
    add_definitions(-DHAVE_HIDAPI)
    include_directories(${HIDAPI_INCLUDE_DIR})
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
# Monero
set(MONERO_PROJECT_BUILD "${MONERO_PROJECT}/build/release" CACHE STRING "Monero project build directory")
message(STATUS "Using monero-project build" : ${MONERO_PROJECT_BUILD})
#[[
add_library(wallet STATIC IMPORTED)
set_target_properties(wallet PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/lib/libwallet.a)
    
# libwallet-crypto.a provides x86_64 asm for some wallet functions
if (EXISTS ${MONERO_PROJECT_BUILD}/src/crypto/wallet/libwallet-crypto.a)
  add_library(wallet_crypto_lib STATIC IMPORTED)
  set_target_properties(wallet_crypto_lib PROPERTIES IMPORTED_LOCATION
      ${MONERO_PROJECT_BUILD}/src/crypto/wallet/libwallet-crypto.a)
  set(wallet_crypto wallet_crypto_lib)
endif()
    
add_library(lmdb STATIC IMPORTED)
set_target_properties(lmdb PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/db_drivers/liblmdb/liblmdb.a)
    
add_library(epee STATIC IMPORTED)
set_target_properties(epee PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/contrib/epee/src/libepee.a)
    
# Unbound
find_package(Unbound)
include_directories(${UNBOUND_INCLUDE_DIR})
if ("${UNBOUND_LIBRARIES}" MATCHES "libunbound")
  message(STATUS "Using libunbound: ${UNBOUND_LIBRARIES}")
  add_library(unbound SHARED IMPORTED)
  set_target_properties(unbound PROPERTIES IMPORTED_LOCATION
      ${UNBOUND_LIBRARIES})
endif()

add_library(rpc_base STATIC IMPORTED)
set_target_properties(rpc_base PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/rpc/librpc_base.a)

add_library(net STATIC IMPORTED)
set_target_properties(net PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/net/libnet.a)
    
add_library(hardforks STATIC IMPORTED)
set_target_properties(hardforks PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/hardforks/libhardforks.a)    
    
add_library(easylogging STATIC IMPORTED)
set_target_properties(easylogging PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/easylogging++/libeasylogging.a)

add_library(cryptonote_core STATIC IMPORTED)
set_target_properties(cryptonote_core PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_core/libcryptonote_core.a)
    
    add_library(cryptonote_protocol STATIC IMPORTED)
set_target_properties(cryptonote_protocol PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_protocol/libcryptonote_protocol.a)

add_library(cryptonote_basic STATIC IMPORTED)
set_target_properties(cryptonote_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_basic/libcryptonote_basic.a)

add_library(cryptonote_format_utils_basic STATIC IMPORTED)
set_target_properties(cryptonote_format_utils_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_basic/libcryptonote_format_utils_basic.a)
    
add_library(mnemonics STATIC IMPORTED)
set_target_properties(mnemonics PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/mnemonics/libmnemonics.a)

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/common/libcommon.a)

add_library(cncrypto STATIC IMPORTED)
set_target_properties(cncrypto PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/crypto/libcncrypto.a)

add_library(ringct STATIC IMPORTED)
set_target_properties(ringct PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/ringct/libringct.a)

add_library(ringct_basic STATIC IMPORTED)
set_target_properties(ringct_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/ringct/libringct_basic.a)

add_library(blockchain_db STATIC IMPORTED)
set_target_properties(blockchain_db PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/blockchain_db/libblockchain_db.a)

add_library(blocks STATIC IMPORTED)
set_target_properties(blocks PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/blocks/libblocks.a)

add_library(checkpoints STATIC IMPORTED)
set_target_properties(checkpoints PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/checkpoints/libcheckpoints.a)

add_library(device STATIC IMPORTED)
set_target_properties(device PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/device/libdevice.a)
    
add_library(device_trezor STATIC IMPORTED)
set_target_properties(device_trezor PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/device_trezor/libdevice_trezor.a)

add_library(multisig STATIC IMPORTED)
set_target_properties(multisig PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/multisig/libmultisig.a)

add_library(version STATIC IMPORTED)
set_target_properties(version PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/libversion.a)
    
add_library(randomx STATIC IMPORTED)
set_target_properties(randomx PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/randomx/librandomx.a)
]]
######################################
set(monero_src wallet rpc_base net lmdb unbound easylogging cryptonote_core cryptonote_protocol cryptonote_basic cryptonote_format_utils_basic mnemonics ringct ringct_basic common cncrypto blockchain_db blocks checkpoints device device_trezor multisig version randomx epee hardforks cncrypto ${wallet_crypto} 
${UNBOUND_LIBRARIES} ${Boost_LIBRARIES} ${Protobuf_LIBRARY} ${LibUSB_LIBRARIES} ${OPENSSL_LIBRARIES} ${SODIUM_LIBRARY} ${HIDAPI_LIBRARY} ${EXTRA_LIBRARIES})
######################################
# monero-cpp (wrapper) v0.5.1 => v0.5.5 => v0.5.10 => v0.6.4 => v0.7.0
include_directories(${neroshop_root}/external/monero-cpp/src/)
set(monero_cpp_src libmonero-cpp.a)#set(monero_cpp_src external/monero-cpp/src/utils/gen_utils.cpp external/monero-cpp/src/utils/monero_utils.cpp external/monero-cpp/src/daemon/monero_daemon_model.cpp external/monero-cpp/src/daemon/monero_daemon.cpp external/monero-cpp/src/wallet/monero_wallet_model.cpp external/monero-cpp/src/wallet/monero_wallet_keys.cpp external/monero-cpp/src/wallet/monero_wallet_full.cpp)

######################################
# libbcrypt (optional)
include_directories(${neroshop_root}/external/libbcrypt)
set(bcrypt_src libbcrypt.a)#set(bcrypt_src external/libbcrypt/crypt_blowfish/crypt_blowfish.c external/libbcrypt/crypt_blowfish/crypt_gensalt.c external/libbcrypt/crypt_blowfish/wrapper.c external/libbcrypt/bcrypt.c)

######################################
# sqlite3 (will be built-in/bundled with neroshop)
#[[find_package(SQLite3)
if(SQLite3_FOUND)
    message(STATUS "Using SQLite3: ${SQLite3_LIBRARY} (v${SQLite3_VERSION})")
    include_directories(${SQLite3_INCLUDE_DIRS})
    set(sqlite_src ${SQLite3_LIBRARY}) # or ${SQLite3_LIBRARIES}
endif()
if(NOT SQLite3_FOUND)
    include_directories(${neroshop_root}/external/sqlite)
    set(sqlite_src ${neroshop_root}/external/sqlite/sqlite3.c) # amalgamation - runs a bit faster
endif()]]
include_directories(${neroshop_root}/external/sqlite)
set(sqlite_src libsqlite3.a)#set(sqlite_src ${neroshop_root}/external/sqlite/sqlite3.c)

######################################
# QR-Code-generator
include_directories(${neroshop_root}/external/QR-Code-generator/cpp)
set(qr_code_generator_src libqrcodegen.a)#set(qr_code_generator_src ${neroshop_root}/external/QR-Code-generator/cpp/qrcodegen.cpp) # or ${neroshop_root}/external/QR-Code-generator/c/qrcodegen.c

######################################
# json
include_directories(${neroshop_root}/external/json/single_include)

######################################
# curl
find_package(CURL) # if curl is found on the system, use the system default shared library, otherwise use the curl that we've built from source
if(CURL_FOUND) 
    message(STATUS "Using CURL: ${CURL_LIBRARIES} (v${CURL_VERSION_STRING})")
    include_directories(${CURL_INCLUDE_DIRS})
    set(curl_src ${CURL_LIBRARIES})
endif()
if(NOT CURL_FOUND)
    include_directories(${neroshop_root}/external/curl/include ${neroshop_root}/external/curl/lib)
    set(curl_src ${neroshop_root}/external/curl/lib/.libs/libcurl.a)
    add_definitions(-DCURL_STATICLIB)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lldap -llber -lnghttp2 -lpsl -lidn2 -lbrotlidec -lzstd -lrtmp") # for Arch (Manjaro) # https://stackoverflow.com/questions/68368557/linking-nghttp2-static-library#comment120830125_68368557
endif()

######################################
# libuv (supports Android :D)
find_package(LibUV)
if(LIBUV_FOUND)
    message(STATUS "Using LibUV: ${LibUV_LIBRARIES} (v${LibUV_VERSION})")
    include_directories(${LibUV_INCLUDE_DIRS})
    set(libuv_src ${LibUV_LIBRARIES})
endif()
if(NOT LIBUV_FOUND)
    include_directories(${neroshop_root}/external/libuv/include ${neroshop_root}/external/libuv/src)
    set(libuv_src ${neroshop_root}/external/libuv/.libs/libuv.a) # In case user chooses to build libuv themselves instead of installing it on the system
endif()

######################################
# willemt/raft (BSD and has zero dependencies :D)
include_directories(${neroshop_root}/external/raft/include)
set(raft_src libraft.a)#set(raft_src ${neroshop_root}/external/raft/src/raft_log.c ${neroshop_root}/external/raft/src/raft_node.c ${neroshop_root}/external/raft/src/raft_server.c ${neroshop_root}/external/raft/src/raft_server_properties.c)

######################################
# stduuid (header-only MIT library for generating uuids)
include_directories(${neroshop_root}/external/stduuid ${neroshop_root}/external/stduuid/catch ${neroshop_root}/external/stduuid/include)

######################################
# linenoise (BSD replacement for GNU readline)
include_directories(${neroshop_root}/external/linenoise)
set(linenoise_src liblinenoise.a)

######################################
# lua
include_directories(${neroshop_root}/external/lua/src) # v5.4.4
set(lua_src liblua.a)

######################################
# png (needs zlib)
include_directories(${neroshop_root}/external/png) # v1.6.37
include_directories(${neroshop_root}/external/zlib) #v1.2.12
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(png_src libpng.so libz.a)
else()
set(png_src libpng.a libz.a)
endif()

######################################
# network_test
set(test_network "network_test")
add_executable(${test_network} network_test.cpp ${neroshop_srcs})
target_link_libraries(${test_network} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
# network_uv test
#[[set(test_network_uv "network_uv_test")
add_executable(${test_network_uv} network_test_uv.cpp ${neroshop_srcs})
target_link_libraries(${test_network_uv} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})]]
# utility test
set(test_utility "utility_test")
add_executable(${test_utility} utility_test.cpp ${neroshop_srcs})
target_link_libraries(${test_utility} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
# auth_test
set(test_auth "auth_test")
add_executable(${test_auth} auth_test.cpp ${neroshop_srcs})
target_link_libraries(${test_auth} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
# uuid_test
set(test_uuid "uuid_test")
add_executable(${test_uuid} uuid_test.cpp)
# qr_code_test
set(test_qr_code "qr_test")
add_executable(${test_qr_code} qr_test.cpp ${neroshop_srcs})
target_link_libraries(${test_qr_code} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
# msg_encryption_test
set(test_crypt "encryption_test")
add_executable(${test_crypt} encryption_test.cpp ${neroshop_srcs})
target_link_libraries(${test_crypt} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
# Qt gui_test
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(test_gui_qt "gui_qt_test")
add_executable(${test_gui_qt} main_window.ui main_window.cpp gui_qt_test.cpp ${neroshop_srcs})
#[[target_]] 
include_directories(#[[${test_gui_qt} ]] /usr/include/x86_64-linux-gnu/qt5/ /usr/include/x86_64-linux-gnu/qt5/QtWidgets/)
#target_compile_definitions(${test_gui_qt} PRIVATE )
target_link_libraries(${test_gui_qt} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})

#[[
set(test_ "")
add_executable(${test_} .cpp ${neroshop_srcs})
target_link_libraries(${test_} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src})
]]

######################################
# Reminder: only tests that use GUI features should link to X11, OpenGL and GLFW
if(DEFINED NEROSHOP_USE_LIBBCRYPT) # -DNEROSHOP_USE_LIBBCRYPT=1
    message(STATUS "Using libbcrypt: ${bcrypt_src}")
    #[[target_compile_definitions(${test_} PUBLIC -DNEROSHOP_USE_LIBBCRYPT)
    target_link_libraries(${test_} ${bcrypt_src})]]
endif()

if(WIN32)
    set(winsock2_src ws2_32.lib) # -lglfw3 -lgdi32 -lopengl32 -limm32
    target_link_libraries(${test_network} ${winsock2_src})
    #target_link_libraries(${test_network_uv} ${winsock2_src})
    #target_link_libraries(${test_} ${winsock2_src})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(posix_src libpthread.so libdl.so libm.so)
    target_link_libraries(${test_network} ${posix_src})
    #target_link_libraries(${test_network_uv} ${posix_src})
    target_link_libraries(${test_utility} ${posix_src})
    target_link_libraries(${test_auth} ${posix_src})
    target_link_libraries(${test_qr_code} ${posix_src})
    target_link_libraries(${test_crypt} ${posix_src})
    target_link_libraries(${test_gui_qt} ${posix_src})
    #target_link_libraries(${test_} ${posix_src})
    find_package(X11 REQUIRED)
    if(X11_FOUND)
	    #target_link_libraries(${test_} ${X11_LIBRARIES})
    endif()
endif()

#[[set(OpenGL_GL_PREFERENCE "LEGACY") # LEGACY (libGL.so) or GLVND (libOpenGL.so and libGLX.so)
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    #target_compile_definitions(${test_} PRIVATE DOKUN_USE_OPENGL)
	#target_link_libraries(${test_} ${OPENGL_LIBRARIES})
endif()]]

# Qt
find_package(Qt5 COMPONENTS Widgets REQUIRED)
if(Qt5_FOUND)
    message(STATUS "Using Qt5 Qt5::Widgets")
    target_link_libraries(${test_gui_qt} Qt5::Widgets)
endif()
# refer to https://doc.qt.io/qt-5/cmake-get-started.html
