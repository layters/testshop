project('neroshop', 'cpp', meson_version: '>= 1.4.1', version: '0.1.0')

# cmake_cxx_standard = '17'
# cmake_cxx_standard_required = 'ON'

# list(['APPEND', 'CMAKE_MODULE_PATH', '${CMAKE_CURRENT_SOURCE_DIR}/cmake', '${CMAKE_CURRENT_SOURCE_DIR}/external/monero-cpp/external/monero-project/cmake'])

######################################
neroshop_src_dir = meson.current_source_dir() + '/src'
neroshop_console_src_dir = neroshop_src_dir + '/console'
neroshop_core_src_dir = neroshop_src_dir + '/core'
neroshop_daemon_src_dir = neroshop_src_dir + '/daemon'
neroshop_gui_src_dir = neroshop_src_dir + '/gui'

######################################
cxx_compiler = meson.get_compiler('cpp')
cxx_flags = ['-fPIC', '-no-pie']

if host_machine.system() == 'linux' or host_machine.system() == 'darwin' or host_machine.system() == 'freebsd'
    pthread_dep = cxx_compiler.find_library('pthread', required: true)
    dl_dep = cxx_compiler.find_library('dl', required: true)
    m_dep = cxx_compiler.find_library('m', required: true)
endif

if host_machine.system() == 'windows'
    winsock2_dep = cxx_compiler.find_library('ws2_32', required: true)
endif

######################################
# extra libraries
extra_dep = []
if host_machine.system() == 'darwin'
    extra_dep = dependency('appleframeworks', modules: ['foundation'])
endif
if host_machine.system() == 'windows'
endif

######################################
# protobuf
protobuf_dep = dependency('protobuf', required: true)#cxx_compiler.find_library('protobuf', required: true)

######################################
# libusb
libusb_names = ['usb-1.0', 'libusb', 'usb']
libusb_dep = dependency('libusb-1.0', required: true)
'''
foreach lib_name : libusb_names
    libusb_dep = dependency(lib_name, required: false)
    if libusb_dep.found()
        break
    endif
endforeach
'''
######################################
# boost
boost_modules = ['system', 'filesystem', 'thread', 'date_time', 'chrono', 'regex', 'serialization', 'program_options', 'locale']
boost_dep = dependency('boost', modules: boost_modules, required: true)

######################################
# openssl
openssl_dep = dependency('openssl', required: true)

######################################
# libsodium
sodium_dep = cxx_compiler.find_library('sodium', required: true)

######################################
# hidapi
hidapi_dep = [cxx_compiler.find_library('udev', required: true), cxx_compiler.find_library('hidapi-libusb', required: true)]

######################################
# unbound
unbound_dep = cxx_compiler.find_library('unbound', required: true)

######################################
# monero
monero_root_dir = 'external/monero-cpp/external/monero-project'#meson.current_source_dir() + '/external/monero-cpp/external/monero-project'
monero_src_dir = monero_root_dir + '/src'
monero_build_dir = monero_root_dir + '/build/release'
monero_inc = [
    monero_root_dir + '/contrib/epee/include',
    monero_root_dir + '/external',
    monero_root_dir + '/external/easylogging++',
    monero_root_dir + '/external/rapidjson/include',
    monero_src_dir + '/',
    monero_src_dir + '/wallet',
    monero_src_dir + '/wallet/api',
    monero_src_dir + '/hardforks',
    monero_src_dir + '/crypto',
    monero_src_dir + '/crypto/crypto_ops_builder/include',
    #protobuf_dep.include_directories(),
    #boost_dep.include_directories(),
    #openssl_dep.include_directories(),
    'external/monero-cpp/external/libsodium/include/sodium',
    #'external/monero-cpp/external/openssl-sdk/include',
    #hidapi_dep.include_directories(),
    #unbound_dep.include_directories()
]

wallet = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/lib', '-lwallet']
wallet_crypto = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/crypto/wallet', '-lwallet-crypto']
lmdb = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/external/db_drivers/liblmdb', '-llmdb']
epee = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/contrib/epee/src', '-lepee']
rpc_base = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/rpc', '-lrpc_base']
net = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/net', '-lnet']
hardforks = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/hardforks', '-lhardforks']
easylogging = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/external/easylogging++', '-leasylogging']
cryptonote_core = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/cryptonote_core', '-lcryptonote_core']
cryptonote_protocol = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/cryptonote_protocol', '-lcryptonote_protocol']
cryptonote_basic = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/cryptonote_basic', '-lcryptonote_basic']
cryptonote_format_utils_basic = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/cryptonote_basic', '-lcryptonote_format_utils_basic']
mnemonics = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/mnemonics', '-lmnemonics']
common = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/common', '-lcommon']
cncrypto = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/crypto', '-lcncrypto']
ringct = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/ringct', '-lringct']
ringct_basic = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/ringct', '-lringct_basic']
blockchain_db = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/blockchain_db', '-lblockchain_db']
blocks = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/blocks', '-lblocks']
checkpoints = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/checkpoints', '-lcheckpoints']
device = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/device', '-ldevice']
device_trezor = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/device_trezor', '-ldevice_trezor']
multisig = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src/multisig', '-lmultisig']
version = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/src', '-lversion']
randomx = ['-L' + meson.current_source_dir() + '/' + monero_build_dir + '/external/randomx', '-lrandomx']

monero_link_args = [wallet, rpc_base, net, lmdb, easylogging, cryptonote_core, cryptonote_protocol, cryptonote_basic, cryptonote_format_utils_basic, mnemonics, ringct, ringct_basic, common, blockchain_db, blocks, checkpoints, device, device_trezor, cncrypto, multisig, version, randomx, epee, hardforks, wallet_crypto]
monero_external_dep = [unbound_dep, boost_dep, protobuf_dep, libusb_dep, openssl_dep, sodium_dep, hidapi_dep, extra_dep]
monero_dep = declare_dependency(
    include_directories: monero_inc,
    link_args: monero_link_args,
    dependencies: monero_external_dep
)

######################################
# monero-cpp
monero_cpp_inc = include_directories('external/monero-cpp/src')#meson.current_source_dir() + '/external/monero-cpp/src'
monero_cpp_dep = declare_dependency(
    include_directories: monero_cpp_inc,
    link_args: ['-lmonero-cpp']
)

######################################
# sqlite3
sqlite3_inc = include_directories('external/sqlite')#meson.current_source_dir() + '/external/sqlite'
sqlite3_dep = declare_dependency(
    include_directories: sqlite3_inc,
    link_args: ['-lsqlite3'],
    version: '3.39.1'
)

######################################
# QR-Code-generator
qrcodegen_inc = include_directories('external/QR-Code-generator/cpp')#meson.current_source_dir() + '/external/QR-Code-generator/cpp'
qrcodegen_dep = declare_dependency(
    include_directories: qrcodegen_inc,
    link_args: ['-lqrcodegen']
)

######################################
# json
json_inc = include_directories('external/json/single_include')#meson.current_source_dir() + '/external/json/single_include'
json_dep = declare_dependency(include_directories: json_inc)

######################################
# curl
curl_inc = []
if get_option('NEROSHOP_BUILD_CLI')
    curl_dep = cxx_compiler.find_library('curl', required: true)
endif

######################################
# stduuid
stduuid_inc = include_directories('external/stduuid', 'external/stduuid/catch', 'external/stduuid/include')
'''
[
    meson.current_source_dir() + '/external/stduuid',
    meson.current_source_dir() + '/external/stduuid/catch',
    meson.current_source_dir() + '/external/stduuid/include'
]
'''
stduuid_dep = declare_dependency(include_directories: stduuid_inc)

######################################
# linenoise
linenoise_inc = include_directories('external/linenoise')#meson.current_source_dir() + '/external/linenoise'
linenoise_dep = declare_dependency(
    include_directories: linenoise_inc,
    link_args: ['-llinenoise']
)

######################################
# lua
lua_inc = include_directories('external/lua/src')#meson.current_source_dir() + '/external/lua/src'
lua_dep = declare_dependency(
    include_directories: lua_inc,
    link_args: ['-llua'],
    version: '5.4.4'
)

######################################
# cxxopts
cxxopts_inc = include_directories('external/cxxopts/include')#meson.current_source_dir() + '/external/cxxopts/include'
cxxopts_dep = declare_dependency(include_directories: cxxopts_inc)    

######################################
# zlib
zlib_inc = include_directories('external/zlib')#meson.current_source_dir() + '/external/zlib'
zlib_dep = declare_dependency(
    include_directories: zlib_inc,
    link_args: ['-lz'],
    version: '1.2.13'
)

######################################
# i2pd
i2pd_inc = include_directories('external/i2pd/daemon', 'external/i2pd/i18n', 'external/i2pd/libi2pd', 'external/i2pd/libi2pd_client')
'''
[
    meson.current_source_dir() + '/external/i2pd/daemon',
    meson.current_source_dir() + '/external/i2pd/i18n',
    meson.current_source_dir() + '/external/i2pd/libi2pd',
    meson.current_source_dir() + '/external/i2pd/libi2pd_client'
]
'''
i2pd_link_args = ['-li2pdclient', '-li2pd', '-li2pdlang']
i2pd_dep = declare_dependency(
    include_directories: i2pd_inc,
    link_args: [i2pd_link_args]
)

i2pd_daemon_dep = declare_dependency(
    include_directories: i2pd_inc,
    link_args: ['-li2pddaemon']
)

######################################
# fmt
fmt_inc = include_directories('external/fmt/include')#meson.current_source_dir() + '/external/fmt/include'
fmt_dep = declare_dependency(
    include_directories: fmt_inc,
    link_args: ['-lfmt']
)

######################################
# qt5
if get_option('NEROSHOP_BUILD_GUI')
    qt5_modules = ['Qml', 'Quick', 'Gui', 'Concurrent']
    qt5_widgets_arg = []
    if host_machine.system() == 'linux'
        qt5_modules += 'Widgets'
        qt5_widgets_arg += '-DNEROSHOP_USE_QT_WIDGETS'
    endif
    qt5 = import('qt5')
    qt5_dep = dependency('qt5', version: '>= 5.15.0', modules: qt5_modules)
    qt5_resources = qt5.compile_resources(sources: ['qml.qrc'], name: 'qrc_qml')
    qt5_headers = files(
        'src/gui/backend.hpp',
        'src/gui/currency_rate_provider.hpp',
        'src/gui/daemon_manager.hpp',
        'src/gui/enum_wrapper.hpp',
        'src/gui/image_loader.hpp',
        'src/gui/proxy_manager.hpp',
        'src/gui/settings_manager.hpp',
        'src/gui/user_manager.hpp',
        'src/gui/wallet_manager.hpp',
    )
    qt5_sources = files(
        'src/gui/backend.cpp',
        'src/gui/currency_rate_provider.cpp',
        'src/gui/daemon_manager.cpp',
        'src/gui/enum_wrapper.cpp',
        'src/gui/image_loader.cpp',
        'src/gui/proxy_manager.cpp',
        'src/gui/settings_manager.cpp',
        'src/gui/user_manager.cpp',
        'src/gui/wallet_manager.cpp',
    )
    moc_output = qt5.compile_moc(sources: qt5_sources, headers: qt5_headers, include_directories: [include_directories('src/gui')], dependencies: qt5_dep)
endif

######################################

neroshop_link_dirs = [meson.current_source_dir() + '/build']
project_link_args = []
foreach d : neroshop_link_dirs
  project_link_args += '-L' + d
endforeach

cxx_flags += project_link_args

neroshop_crypto_srcs = [
    neroshop_core_src_dir + '/crypto/rsa.cpp',
    neroshop_core_src_dir + '/crypto/sha256.cpp',
    neroshop_core_src_dir + '/crypto/sha3.cpp'
]

neroshop_database_srcs = [
    neroshop_core_src_dir + '/database/database.cpp',
    neroshop_core_src_dir + '/database/sqlite3/sqlite3.cpp'
]

neroshop_network_srcs = [
    neroshop_core_src_dir + '/network/i2p.cpp'
]

neroshop_price_srcs = [
    neroshop_core_src_dir + '/price/coincodex/coincodex.cpp',
    neroshop_core_src_dir + '/price/coingecko/coingecko.cpp',
    neroshop_core_src_dir + '/price/coinmarketcap/coinmarketcap.cpp',
    neroshop_core_src_dir + '/price/cointelegraph/cointelegraph.cpp',
    neroshop_core_src_dir + '/price/cryptorank/cryptorank.cpp',
    neroshop_core_src_dir + '/price/cryptowatch/cryptowatch.cpp',
    neroshop_core_src_dir + '/price/fawazahmed0/fawazahmed0.cpp',
    neroshop_core_src_dir + '/price/kraken/kraken.cpp',
    neroshop_core_src_dir + '/price/currency_converter.cpp'
]

neroshop_protocol_srcs = [
    neroshop_core_src_dir + '/protocol/p2p/file_piece_hasher.cpp',
    neroshop_core_src_dir + '/protocol/p2p/key_mapper.cpp',
    neroshop_core_src_dir + '/protocol/p2p/node.cpp',
    neroshop_core_src_dir + '/protocol/p2p/routing_table.cpp',
    neroshop_core_src_dir + '/protocol/p2p/serializer.cpp',
    neroshop_core_src_dir + '/protocol/rpc/json_rpc.cpp',
    neroshop_core_src_dir + '/protocol/rpc/msgpack.cpp',
    neroshop_core_src_dir + '/protocol/transport/client.cpp',
    neroshop_core_src_dir + '/protocol/transport/sam_client.cpp',
    neroshop_core_src_dir + '/protocol/transport/server.cpp'
]

neroshop_tools_srcs = [
    neroshop_core_src_dir + '/tools/base64.cpp',
    neroshop_core_src_dir + '/tools/process.cpp',
    neroshop_core_src_dir + '/tools/script.cpp',
    neroshop_core_src_dir + '/tools/timestamp.cpp',
]

neroshop_wallet_srcs = [
    neroshop_core_src_dir + '/wallet/wallet.cpp'
]

neroshop_client_srcs = [
    neroshop_crypto_srcs,
    neroshop_database_srcs,
    neroshop_network_srcs,
    neroshop_price_srcs,
    neroshop_core_src_dir + '/protocol/p2p/file_piece_hasher.cpp',
    neroshop_core_src_dir + '/protocol/p2p/serializer.cpp',
    neroshop_core_src_dir + '/protocol/rpc/json_rpc.cpp',
    neroshop_core_src_dir + '/protocol/transport/client.cpp',
    neroshop_tools_srcs,
    neroshop_wallet_srcs,
    neroshop_core_src_dir + '/cart.cpp',
    neroshop_core_src_dir + '/escrow.cpp',
    neroshop_core_src_dir + '/listing.cpp',
    neroshop_core_src_dir + '/order.cpp',
    neroshop_core_src_dir + '/product.cpp',
    neroshop_core_src_dir + '/seller.cpp',
    neroshop_core_src_dir + '/settings.cpp',
    neroshop_core_src_dir + '/user.cpp'
]

if get_option('NEROSHOP_BUILD_TESTS')
    message('NEROSHOP_BUILD_TESTS option set to true')
    #subdir('tests')
endif

neroshop_daemon = 'neroshopd'
neroshop_daemon_srcs = [
    neroshop_daemon_src_dir + '/main.cpp',
    neroshop_crypto_srcs,
    neroshop_database_srcs,
    neroshop_network_srcs,
    neroshop_core_src_dir + '/protocol/p2p/file_piece_hasher.cpp',
    neroshop_core_src_dir + '/protocol/p2p/key_mapper.cpp',
    neroshop_core_src_dir + '/protocol/p2p/node.cpp',
    neroshop_core_src_dir + '/protocol/p2p/routing_table.cpp',
    neroshop_core_src_dir + '/protocol/rpc/json_rpc.cpp',
    neroshop_core_src_dir + '/protocol/rpc/msgpack.cpp',
    neroshop_core_src_dir + '/protocol/transport/client.cpp',
    neroshop_core_src_dir + '/protocol/transport/sam_client.cpp',
    neroshop_core_src_dir + '/protocol/transport/server.cpp',
    neroshop_core_src_dir + '/tools/base64.cpp',
    neroshop_core_src_dir + '/tools/timestamp.cpp'
]
executable(neroshop_daemon, neroshop_daemon_srcs,
    include_directories: [monero_inc, monero_cpp_inc, sqlite3_inc, json_inc, zlib_inc, i2pd_inc, fmt_inc, cxxopts_inc],
    cpp_args: ['-DNEROSHOP_DEBUG', '-DNEROSHOP_USE_SYSTEM_SOCKETS'],
    link_args: cxx_flags,
    dependencies: [i2pd_dep, i2pd_daemon_dep, monero_cpp_dep, sqlite3_dep, monero_dep, zlib_dep, fmt_dep, pthread_dep, dl_dep, m_dep]
)

if get_option('NEROSHOP_BUILD_CLI')
    neroshop_cli = 'neroshop-console'
    neroshop_cli_srcs = [neroshop_console_src_dir + '/main.cpp'] + neroshop_client_srcs
    executable(neroshop_cli, neroshop_cli_srcs,
        include_directories: [monero_inc, monero_cpp_inc, sqlite3_inc, json_inc, zlib_inc, i2pd_inc, fmt_inc, lua_inc, curl_inc, stduuid_inc, linenoise_inc],
        cpp_args: ['-DNEROSHOP_DEBUG', '-DNEROSHOP_USE_SYSTEM_SOCKETS', '-DNEROSHOP_BUILD_CLI'],
        link_args: cxx_flags,
        dependencies: [i2pd_dep, monero_cpp_dep, sqlite3_dep, curl_dep, monero_dep, zlib_dep, fmt_dep, lua_dep, linenoise_dep, pthread_dep, dl_dep, m_dep]
    )
endif

if get_option('NEROSHOP_BUILD_GUI')
    neroshop_gui = 'neroshop'
    neroshop_gui_srcs = qt5_resources + moc_output + [neroshop_gui_src_dir + '/main.cpp', 'src/gui/settings_manager.cpp', 'src/gui/wallet_manager.cpp', 'src/gui/user_manager.cpp', 'src/gui/image_loader.cpp', 'src/gui/image_provider.cpp', 'src/gui/wallet_qr_provider.cpp', 'src/gui/currency_rate_provider.cpp', 'src/gui/daemon_manager.cpp', 'src/gui/enum_wrapper.cpp', 'src/gui/proxy_manager.cpp', 'src/gui/backend.cpp'] + neroshop_client_srcs
    executable(neroshop_gui, neroshop_gui_srcs,
        include_directories: [monero_inc, monero_cpp_inc, sqlite3_inc, json_inc, zlib_inc, i2pd_inc, fmt_inc, lua_inc, qrcodegen_inc],
        cpp_args: ['-DNEROSHOP_DEBUG', '-DNEROSHOP_USE_SYSTEM_SOCKETS', '-DNEROSHOP_BUILD_GUI', '-DNEROSHOP_USE_QT'] + qt5_widgets_arg,
        link_args: cxx_flags,
        dependencies: [i2pd_dep, monero_cpp_dep, sqlite3_dep, qrcodegen_dep, monero_dep, zlib_dep, fmt_dep, lua_dep, qt5_dep, pthread_dep, dl_dep, m_dep]
    )
endif

# sudo rm -rf build/meson-info build/meson-logs build/meson-private build/build.ninja build/compile_commands.json build/.ninja_deps build/.ninja_log
# meson setup --reconfigure build -DNEROSHOP_BUILD_CLI=true
# cd build
# meson compile
