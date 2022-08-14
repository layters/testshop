cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(neroshop) # tested on Ubuntu 20.04 LTS (focal) with gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0, manjaro-xfce-21.2.6 (Qonos) with gcc (GCC) 11.2.0

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

option(NEROSHOP_BUILD_TESTS "Build neroshop tests" OFF)
option(NEROSHOP_BUILD_CLI "Build neroshop CLI" ON) # CLI is for testing only. There's no reason for it besides that. This is an ecommerce app so a GUI is ALWAYS necessary!
option(NEROSHOP_BUILD_GUI "Build neroshop GUI" OFF)
option(UUID_SYSTEM_GENERATOR "Enable operating system uuid generator" OFF)
option(UUID_TIME_GENERATOR "Enable experimental time-based uuid generator" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/monero-project/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/cmake")
######################################
if(WIN32)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# Set compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w") #-w=ignores all warnings
    message(STATUS "Building with clang")
else()
    message(STATUS "Building with gcc/g++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie") # detach
endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif()

######################################
set(MONERO_PROJECT "${CMAKE_SOURCE_DIR}/external/monero-cpp/external/monero-project")
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

set(EXTERNAL_LIBS_DIR ${CMAKE_SOURCE_DIR}/monero-cpp/external-libs)
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
# Monero (monero-0.17.2.3 contents pasted in "monero-cpp/external/monero-project")
set(MONERO_PROJECT_BUILD "${MONERO_PROJECT}/build/release")
message(STATUS "Using monero-project build" : ${MONERO_PROJECT_BUILD})

add_library(wallet_merged STATIC IMPORTED)
set_target_properties(wallet_merged PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/lib/libwallet_merged.a)
    
add_library(wallet STATIC IMPORTED)
set_target_properties(wallet PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/lib/libwallet.a)
    
add_library(wallet_crypto STATIC IMPORTED)
set_target_properties(wallet_crypto PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/crypto/wallet/libwallet-crypto.a)
    
add_library(lmdb STATIC IMPORTED)
set_target_properties(lmdb PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/db_drivers/liblmdb/liblmdb.a)
    
add_library(epee STATIC IMPORTED)
set_target_properties(epee PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/contrib/epee/src/libepee.a)
    
add_library(unbound STATIC IMPORTED)
set_target_properties(unbound PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/unbound/libunbound.a)
    
add_library(easylogging STATIC IMPORTED)
set_target_properties(easylogging PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/external/easylogging++/libeasylogging.a)

add_library(cryptonote_core STATIC IMPORTED)
set_target_properties(cryptonote_core PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_core/libcryptonote_core.a)

add_library(cryptonote_basic STATIC IMPORTED)
set_target_properties(cryptonote_basic PROPERTIES IMPORTED_LOCATION
    ${MONERO_PROJECT_BUILD}/src/cryptonote_basic/libcryptonote_basic.a)
# new addition [2021-10-02]
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
######################################
set(monero_src wallet_merged wallet_crypto #[[wallet_api]] wallet lmdb epee unbound easylogging cryptonote_core cryptonote_basic cryptonote_format_utils_basic mnemonics ringct ringct_basic common cncrypto blockchain_db blocks checkpoints device device_trezor multisig version randomx 
${Boost_LIBRARIES} ${Protobuf_LIBRARY} ${LibUSB_LIBRARIES} ${OPENSSL_LIBRARIES} ${SODIUM_LIBRARY} ${HIDAPI_LIBRARY} ${EXTRA_LIBRARIES})
######################################
# monero-cpp (wrapper)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/src/)
set(monero_cpp_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libmonero-cpp.a)#set(monero_cpp_src external/monero-cpp/src/utils/gen_utils.cpp external/monero-cpp/src/utils/monero_utils.cpp external/monero-cpp/src/daemon/monero_daemon_model.cpp external/monero-cpp/src/daemon/monero_daemon.cpp external/monero-cpp/src/wallet/monero_wallet_model.cpp external/monero-cpp/src/wallet/monero_wallet_keys.cpp external/monero-cpp/src/wallet/monero_wallet_full.cpp)

######################################
# libbcrypt (optional)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libbcrypt)
set(bcrypt_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libbcrypt.a)#set(bcrypt_src external/libbcrypt/crypt_blowfish/crypt_blowfish.c external/libbcrypt/crypt_blowfish/crypt_gensalt.c external/libbcrypt/crypt_blowfish/wrapper.c external/libbcrypt/bcrypt.c)

######################################
# sqlite3 (will be built-in/bundled with neroshop)
#[[find_package(SQLite3)
if(SQLite3_FOUND)
    message(STATUS "Using SQLite3: ${SQLite3_LIBRARY} (v${SQLite3_VERSION})")
    include_directories(${SQLite3_INCLUDE_DIRS})
    set(sqlite_src ${SQLite3_LIBRARY}) # or ${SQLite3_LIBRARIES}
endif()
if(NOT SQLite3_FOUND)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite)
    set(sqlite_src ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite/sqlite3.c) # amalgamation - runs a bit faster
endif()]]
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite)
set(sqlite_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libsqlite3.a)#set(sqlite_src ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite/sqlite3.c)

######################################
# QR-Code-generator
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QR-Code-generator/cpp)
set(qr_code_generator_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libqrcodegen.a)#set(qr_code_generator_src ${CMAKE_CURRENT_SOURCE_DIR}/external/QR-Code-generator/cpp/qrcodegen.cpp) # or ${CMAKE_CURRENT_SOURCE_DIR}/external/QR-Code-generator/c/qrcodegen.c

######################################
# json
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include)

######################################
# curl
find_package(CURL) # if curl is found on the system, use the system default shared library, otherwise use the curl that we've built from source
if(CURL_FOUND) 
    message(STATUS "Using CURL: ${CURL_LIBRARIES} (v${CURL_VERSION_STRING})")
    include_directories(${CURL_INCLUDE_DIRS})
    set(curl_src ${CURL_LIBRARIES})
endif()
if(NOT CURL_FOUND)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib)
    set(curl_src ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib/.libs/libcurl.a)
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
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libuv/include ${CMAKE_CURRENT_SOURCE_DIR}/external/libuv/src)
    set(libuv_src ${CMAKE_CURRENT_SOURCE_DIR}/external/libuv/.libs/libuv.a) # In case user chooses to build libuv themselves instead of installing it on the system
endif()

######################################
# willemt/raft (BSD and has zero dependencies :D)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/raft/include)
set(raft_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libraft.a)#set(raft_src ${CMAKE_CURRENT_SOURCE_DIR}/external/raft/src/raft_log.c ${CMAKE_CURRENT_SOURCE_DIR}/external/raft/src/raft_node.c ${CMAKE_CURRENT_SOURCE_DIR}/external/raft/src/raft_server.c ${CMAKE_CURRENT_SOURCE_DIR}/external/raft/src/raft_server_properties.c)

######################################
# stduuid (header-only MIT library for generating uuids)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/catch ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/include)

######################################
# linenoise (BSD replacement for GNU readline)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/linenoise)
set(linenoise_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblinenoise.a)

######################################
# lua
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/lua/src) # v5.4.4
set(lua_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblua.a)

######################################
# png (needs zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/png) # v1.6.37
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/zlib) #v1.2.12
set(png_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libpng.a ${CMAKE_CURRENT_SOURCE_DIR}/build/libz.a)

######################################
# dokun-ui_dependencies
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/lua/lua-5.4.1/src) # lua
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/glew/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/png ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/freetype/include                   # public headers - ft2build.h
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/freetype/include/freetype           # public headers
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/freetype/include/freetype/config    # public config headers
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/freetype/include/freetype/internal) # private headers - added 2019-07-29
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/ogg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/vorbis/include
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/vorbis/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/theora/include)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/utf8/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/openal/include
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/openal-soft/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/jpeg
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/jpeg/jpeg-6b)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/tiff/libtiff)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/gif/lib)
include_directories(#[[${CMAKE_CURRENT_SOURCE_DIR}/external/flac]] # config.h
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/flac/include
   ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/flac/src/libFLAC/include) # for "private/cpu.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/external/glm)
######################################
# dokun-ui
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/include)
set(dokun_ui_src ${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui/build/libdokun-ui.a) # definitions added: DOKUN_OPENGL, DOKUN_BUILD_LIBRARY#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/dokun-ui)

######################################
# global directories
include_directories(#[[${CMAKE_CURRENT_SOURCE_DIR}/src/]] ${CMAKE_CURRENT_SOURCE_DIR}/external)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build)
######################################
# neroshop tests
if(NEROSHOP_BUILD_TESTS) # -DNEROSHOP_BUILD_TESTS=1
    enable_testing()
    message(STATUS "NEROSHOP_BUILD_TESTS option set to ON")
    add_subdirectory(test)
endif()

######################################
# neroshop-daemon
set(daemon_executable "neromon")
add_executable(${daemon_executable} src/daemon.cpp src/server.cpp)#target_link_libraries(daemon ${curl_src} ${OPENSSL_LIBRARIES}) # curl requires both openssl(used in monero) and zlib(used in dokun-ui)
target_link_libraries(${daemon_executable} ${libuv_src})#set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl") # fixes undefined reference to symbol 'dlsym@@GLIBC_2.2.5' error

######################################
# neroshop
set(neroshop_executable "neroshop")
set(neroshop_src src/buyer.cpp #[[src/carrier.cpp]] src/cart.cpp src/catalog.cpp src/client.cpp src/converter.cpp src/database.cpp src/encryptor.cpp #[[src/icon.cpp]] src/item.cpp #[[src/main.cpp]] #[[src/message.cpp]] src/order.cpp src/process.cpp src/qr.cpp src/script.cpp src/seller.cpp src/server.cpp src/user.cpp src/validator.cpp src/wallet.cpp)
if(NEROSHOP_BUILD_CLI)
    list(APPEND neroshop_src src/console/main.cpp)
endif()
if(NEROSHOP_BUILD_GUI)
    list(APPEND neroshop_src src/gui/icon.cpp src/gui/main.cpp src/gui/message.cpp)
endif()
add_executable(${neroshop_executable} ${neroshop_src})
target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_DEBUG) # for more detailed debugging, use -DNEROSHOP_DEBUG0 # set_target_properties(${neroshop_executable} PROPERTIES COMPILE_DEFINITIONS "NEROSHOP_DEBUG")
target_link_libraries(${neroshop_executable} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src})
# todo: create separate targets - neroshop-gui and neroshop-cli
if(NEROSHOP_BUILD_CLI)
    target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_BUILD_CLI)
    target_link_libraries(${neroshop_executable} ${linenoise_src} ${lua_src} ${png_src}) # no dokun-ui in CLI so we have to link to lua (for config scripts) and png (for qr code export) themselves
endif()
if(NEROSHOP_BUILD_GUI)
    target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_BUILD_GUI)
    # link GUI library here eg: QT, dokun-ui, etc.
    #target_link_libraries(${neroshop_executable} ${dokun_ui_src}) # If using dokun-ui for the user interface, you will not need to link to png nor lua
    #target_link_libraries(${neroshop_executable} )
endif()

######################################
# note: currently the CLI depends on dokun-ui for Window, File, Script (config.h), libpng (qr.cpp), lua (script.cpp), etc. (catalog.cpp also uses GUI features)
if(DEFINED NEROSHOP_USE_LIBBCRYPT) # -DNEROSHOP_USE_LIBBCRYPT=1
    message(STATUS "Using libbcrypt: ${bcrypt_src}")
    target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_USE_LIBBCRYPT)
    target_link_libraries(${neroshop_executable} ${bcrypt_src})
endif()

if (UUID_TIME_GENERATOR) # -DUUID_TIME_GENERATOR=1
    target_compile_definitions(${neroshop_executable} PRIVATE UUID_TIME_GENERATOR)
endif()

if(WIN32)
    set(winsock2_src ws2_32.lib) # -lglfw3 -lgdi32 -lopengl32 -limm32
    target_link_libraries(${neroshop_executable} ${winsock2_src})
    target_link_libraries(${daemon_executable} ${winsock2_src})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(LibUUID) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        if (LIBUUID_FOUND)
            message(STATUS "Using LibUUID: ${LibUUID_LIBRARY}")
            target_compile_definitions(${neroshop_executable} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
            target_link_libraries(${neroshop_executable} ${LibUUID_LIBRARY})
        endif()
    endif()

    set(posix_src libpthread.so libdl.so libm.so)
    target_link_libraries(${neroshop_executable} ${posix_src})
    target_link_libraries(${daemon_executable} ${posix_src})
    find_package(X11 REQUIRED)
    if(X11_FOUND)
    if(NEROSHOP_BUILD_GUI)
	    target_link_libraries(${neroshop_executable} ${X11_LIBRARIES})
	endif()
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(CFLIB CoreFoundation) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        message(STATUS "Using CFLIB: ${CFLIB}")
        target_compile_definitions(${neroshop_executable} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
        target_link_libraries(${neroshop_executable} ${CFLIB})
    endif()
endif()

set(OpenGL_GL_PREFERENCE "LEGACY") # LEGACY (libGL.so) or GLVND (libOpenGL.so and libGLX.so)
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
if(NEROSHOP_BUILD_GUI)
    target_compile_definitions(${neroshop_executable} PRIVATE DOKUN_USE_OPENGL)
	target_link_libraries(${neroshop_executable} ${OPENGL_LIBRARIES})
endif()
endif()

find_package(GLFW3)# REQUIRED)
if(GLFW3_FOUND)
if(NEROSHOP_BUILD_GUI)
	target_compile_definitions(${neroshop_executable} PRIVATE DOKUN_USE_GLFW) # already defined in libdokun-ui.a
    target_link_libraries(${neroshop_executable} ${GLFW3_LIBRARY})
endif()
endif()

######################################
