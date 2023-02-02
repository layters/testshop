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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/monero-project/cmake")
######################################
if(NEROSHOP_BUILD_GUI AND NEROSHOP_USE_QT)
    set(CMAKE_AUTOMOC ON)
    #set(CMAKE_AUTORCC ON)
    #set(CMAKE_AUTOUIC ON)
    if(CMAKE_VERSION VERSION_LESS "3.7.0")
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()
    find_package(Qt5 COMPONENTS Qml Quick Gui Concurrent #[[QuickControls2]])# REQUIRED) # v5.12.8
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
    set(BoldRed     "${Esc}[1;31m")
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -no-pie")# -fsanitize=address") # -no-pie=can run outside terminal
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
include_directories("${MONERO_PROJECT_SRC}/hardforks")
include_directories("${MONERO_PROJECT_SRC}/crypto/crypto_ops_builder/include/")

set(EXTERNAL_LIBS_DIR ${CMAKE_SOURCE_DIR}/monero-cpp/external-libs)
message(STATUS EXTERNAL_LIBS_DIR : ${EXTERNAL_LIBS_DIR})
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
include_directories(${Protobuf_INCLUDE_DIR})

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
include_directories(${Boost_INCLUDE_DIR})

######################################
# OpenSSL
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
include_directories(external/monero-cpp/external/libsodium/include/sodium) # "export.h"
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

#list(APPEND CMAKE_MODULE_PATH "${MONERO_PROJECT}/cmake")
    
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
    
######################################
set(monero_src wallet rpc_base net lmdb unbound easylogging cryptonote_core cryptonote_protocol cryptonote_basic cryptonote_format_utils_basic mnemonics ringct ringct_basic common cncrypto blockchain_db blocks checkpoints device device_trezor multisig version randomx epee hardforks cncrypto ${wallet_crypto} 
${UNBOUND_LIBRARIES} ${Boost_LIBRARIES} ${Protobuf_LIBRARY} ${LibUSB_LIBRARIES} ${OPENSSL_LIBRARIES} ${SODIUM_LIBRARY} ${HIDAPI_LIBRARY} ${EXTRA_LIBRARIES})
######################################
# monero-cpp (wrapper)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/src/)
set(monero_cpp_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libmonero-cpp.a)

######################################
# libbcrypt (optional)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libbcrypt)
set(bcrypt_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libbcrypt.a)

######################################
# sqlite3
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite) # v3.39.1 (amalgamation - said to run a bit faster)
set(sqlite_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libsqlite3.a)

######################################
# QR-Code-generator
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QR-Code-generator/cpp) # v1.8.0
set(qr_code_generator_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libqrcodegen.a)

######################################
# json
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include)

######################################
# curl
find_package(CURL)
if(CURL_FOUND) 
    message(STATUS "Using CURL: ${CURL_LIBRARIES} (v${CURL_VERSION_STRING})")
    include_directories(${CURL_INCLUDE_DIRS})
    set(curl_src ${CURL_LIBRARIES})
endif()
if(NOT CURL_FOUND)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib)
    set(curl_src ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib/.libs/libcurl.a) # In case user chooses to build libcurl themselves instead of installing it on the system
    add_definitions(-DCURL_STATICLIB)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lldap -llber -lnghttp2 -lpsl -lidn2 -lbrotlidec -lzstd -lrtmp") # for Arch (Manjaro)
endif()

######################################
# libuv
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
# willemt/raft
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/raft/include)
set(raft_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libraft.a)

######################################
# stduuid
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/catch ${CMAKE_CURRENT_SOURCE_DIR}/external/stduuid/include)

######################################
# linenoise
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/linenoise/)
set(linenoise_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblinenoise.a)

######################################
# lua
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/lua/src/) # v5.4.4
set(lua_src ${CMAKE_CURRENT_SOURCE_DIR}/build/liblua.a)

######################################
# png (needs zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/png/) # v1.6.37
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/) #v1.2.12
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(png_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libpng.so ${CMAKE_CURRENT_SOURCE_DIR}/build/libz.a) # Linux requires libpng to be built dynamically to prevent the Gtk:ERROR: ... "Failed to load /usr/share/icons/Yaru/16x16/status/image-missing.png: Fatal error reading PNG image file: Invalid IHDR data (gdk-pixbuf-error-quark, 0)" when opening the QML FileDialog
else()
    set(png_src ${CMAKE_CURRENT_SOURCE_DIR}/build/libpng.a ${CMAKE_CURRENT_SOURCE_DIR}/build/libz.a)
endif()

######################################
# neroshop link directories
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build) # target_link_directories only available in CMake version 3.13 :(

######################################
# neroshop core source files
set(neroshop_core_src src/core/buyer.cpp src/core/cart.cpp src/core/catalog.cpp src/core/client.cpp src/core/config.cpp src/core/currency_converter.cpp src/core/database.cpp src/core/encryptor.cpp src/core/item.cpp src/core/order.cpp src/core/process.cpp src/core/qr.cpp src/core/script.cpp src/core/seller.cpp src/core/server.cpp src/core/user.cpp src/core/validator.cpp src/core/wallet.cpp)

######################################
# neroshop tests
if(NEROSHOP_BUILD_TESTS) # -DNEROSHOP_BUILD_TESTS=1
    enable_testing()
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_TESTS option set to ON${ColourReset}")
    add_subdirectory(test)
endif()

######################################
# neroshop-daemon
set(daemon_executable "neromon")
add_executable(${daemon_executable} src/daemon/main.cpp src/core/server.cpp)#target_link_libraries(daemon ${curl_src} ${OPENSSL_LIBRARIES}) # curl requires both openssl(used in monero) and zlib(used in dokun-ui)
target_include_directories(${daemon_executable} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]])
target_link_libraries(${daemon_executable} ${libuv_src} ${raft_src})#set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl") # fixes undefined reference to symbol 'dlsym@@GLIBC_2.2.5' error

######################################
# neroshop-console
if(NEROSHOP_BUILD_CLI)
    set(neroshop_console "neroshop-console")
    add_executable(${neroshop_console} src/console/main.cpp ${neroshop_core_src})
    target_compile_definitions(${neroshop_console} PRIVATE NEROSHOP_DEBUG)
    target_include_directories(${neroshop_console} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]])
    target_link_libraries(${neroshop_console} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${curl_src} ${monero_src} ${lua_src} ${png_src} ${linenoise_src})
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_CLI option set to ON${ColourReset}")
endif()

######################################
# neroshop
if(NEROSHOP_BUILD_GUI)
    set(neroshop_executable "neroshop")
    set(neroshop_core_src ${neroshop_core_src} src/core/price_api/price_api_factory.cpp src/core/price_api/coincodex.cpp src/core/price_api/coingecko.cpp src/core/price_api/coinmarketcap.cpp src/core/price_api/cointelegraph.cpp src/core/price_api/cryptorank.cpp src/core/price_api/cryptowatch.cpp)
    add_executable(${neroshop_executable})
    target_sources(${neroshop_executable} PRIVATE ${neroshop_res} src/gui/main.cpp src/gui/script_controller.cpp src/gui/wallet_controller.cpp src/gui/user_controller.cpp src/gui/image_loader.cpp src/gui/image_provider.cpp src/gui/wallet_qr_provider.cpp src/gui/currencyexchangeratesprovider.cpp src/gui/backend.cpp ${neroshop_core_src})
    target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_DEBUG)#set_target_properties(${neroshop_executable} PROPERTIES COMPILE_DEFINITIONS "NEROSHOP_DEBUG")
    target_include_directories(${neroshop_executable} PRIVATE #[[${CMAKE_CURRENT_SOURCE_DIR}/src/]])
    target_link_libraries(${neroshop_executable} ${monero_cpp_src} ${sqlite_src} ${qr_code_generator_src} ${raft_src} ${libuv_src} ${monero_src} ${lua_src} ${png_src})
    message(STATUS "${BoldMagenta}NEROSHOP_BUILD_GUI option set to ON${ColourReset}")
    target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_BUILD_GUI)
endif()

######################################
if(DEFINED NEROSHOP_USE_LIBBCRYPT) # -DNEROSHOP_USE_LIBBCRYPT=1
    message(STATUS "Using libbcrypt: ${bcrypt_src}")
    if(NEROSHOP_BUILD_GUI)
        target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_USE_LIBBCRYPT)
        target_link_libraries(${neroshop_executable} ${bcrypt_src})
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_compile_definitions(${neroshop_console} PRIVATE NEROSHOP_USE_LIBBCRYPT)
        target_link_libraries(${neroshop_console} ${bcrypt_src})
    endif()    
endif()

if (UUID_TIME_GENERATOR) # -DUUID_TIME_GENERATOR=1
    if(NEROSHOP_BUILD_GUI)
        target_compile_definitions(${neroshop_executable} PRIVATE UUID_TIME_GENERATOR)
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_compile_definitions(${neroshop_console} PRIVATE UUID_TIME_GENERATOR)
    endif()    
endif()

if(WIN32)
    set(winsock2_src ws2_32.lib) # -lglfw3 -lgdi32 -lopengl32 -limm32
    if(NEROSHOP_BUILD_GUI)
        target_link_libraries(${neroshop_executable} ${winsock2_src})
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_link_libraries(${neroshop_console} ${winsock2_src})
    endif()    
    target_link_libraries(${daemon_executable} ${winsock2_src})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(LibUUID) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        if (LIBUUID_FOUND)
            message(STATUS "Using LibUUID: ${LibUUID_LIBRARY}")
            if(NEROSHOP_BUILD_GUI)
                target_compile_definitions(${neroshop_executable} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
                target_link_libraries(${neroshop_executable} ${LibUUID_LIBRARY})
            endif()
            if(NEROSHOP_BUILD_CLI)
                target_compile_definitions(${neroshop_console} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
                target_link_libraries(${neroshop_console} ${LibUUID_LIBRARY})        
            endif()    
        endif()
    endif()

    set(posix_src libpthread.so libdl.so libm.so)
    if(NEROSHOP_BUILD_GUI)
        target_link_libraries(${neroshop_executable} ${posix_src})
    endif()
    if(NEROSHOP_BUILD_CLI)
        target_link_libraries(${neroshop_console} ${posix_src})
    endif()
    target_link_libraries(${daemon_executable} ${posix_src})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(CFLIB CoreFoundation) # optional
    if(UUID_SYSTEM_GENERATOR) # -DUUID_SYSTEM_GENERATOR=1
        message(STATUS "UUID_SYSTEM_GENERATOR option set to ON")
        message(STATUS "Using CFLIB: ${CFLIB}")
        if(NEROSHOP_BUILD_GUI)
            target_compile_definitions(${neroshop_executable} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
            target_link_libraries(${neroshop_executable} ${CFLIB})
        endif()        
        if(NEROSHOP_BUILD_CLI)
            target_compile_definitions(${neroshop_console} PRIVATE UUID_SYSTEM_GENERATOR) # uuids::uuid_system_generator
            target_link_libraries(${neroshop_console} ${CFLIB})
        endif()
    endif()
endif()

if(Qt5_FOUND)
    if(NEROSHOP_BUILD_GUI AND NEROSHOP_USE_QT)
        # Some Linux distros require Qt::Widgets due to the lack of a native platform file dialog implementation
        if(NEROSHOP_USE_QT_WIDGETS OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            message(STATUS "Using Qt5::Widgets")
            target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_USE_QT_WIDGETS)
            find_package(Qt5 COMPONENTS Widgets)
            target_link_libraries(${neroshop_executable} Qt5::Widgets)
        endif()
    
        message(STATUS "Using Qt5::Qml Qt5::Quick Qt5::Gui Qt5::Concurrent")
        target_compile_definitions(${neroshop_executable} PRIVATE NEROSHOP_USE_QT)
        #add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00) # Disable any API deprecated in 5.15 or older Qt versions
        #target_include_directories(${neroshop_executable} PRIVATE /usr/include/x86_64-linux-gnu/qt5/)
        target_link_libraries(${neroshop_executable} Qt5::Qml Qt5::Quick Qt5::Gui Qt5::Concurrent #[[Qt5::QuickControls2]])
    endif()
endif()

######################################
