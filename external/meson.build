project('neroshop external', ['c', 'cpp'], default_options : ['default_library=static'])

# Part of this meson build was generated using 'meson/tools/cmake2meson.py'

monero_include_dirs = ['monero-cpp/external/monero-project/contrib/epee/include/', 'monero-cpp/external/monero-project/external/easylogging++/', 'monero-cpp/external/monero-project/src/', 'monero-cpp/external/monero-project/external/rapidjson/include/', 'monero-cpp/external/monero-project/external/']
monero_cpp = 'monero-cpp'
monero_cpp_srcs = ['monero-cpp/src/utils/gen_utils.cpp', 'monero-cpp/src/utils/monero_utils.cpp', 'monero-cpp/src/daemon/monero_daemon_model.cpp', 'monero-cpp/src/daemon/monero_daemon.cpp', 'monero-cpp/src/wallet/monero_wallet_model.cpp', 'monero-cpp/src/wallet/monero_wallet_keys.cpp', 'monero-cpp/src/wallet/monero_wallet_full.cpp']
monero_cpp_lib = static_library(monero_cpp, monero_cpp_srcs,
    include_directories: ['monero-cpp/src/', monero_include_dirs],
    cpp_args: '-DBOOST_DISABLE_PRAGMA_MESSAGE'
)

qrcodegen = 'qrcodegen'
qr_code_generator_srcs = 'QR-Code-generator/cpp/qrcodegen.cpp'
qrcodegen_lib = static_library(qrcodegen, qr_code_generator_srcs,
    include_directories: 'QR-Code-generator/cpp/'
)

sqlite3 = 'sqlite3'
sqlite_srcs = 'sqlite/sqlite3.c'
sqlite3_lib = static_library(sqlite3, sqlite_srcs,
    include_directories: 'sqlite/',
    c_args: '-DSQLITE_ENABLE_FTS5'
)

linenoise = 'linenoise'
linenoise_srcs = 'linenoise/linenoise.c'
linenoise_lib = static_library(linenoise, linenoise_srcs,
    include_directories: 'linenoise/'
)

lua = 'lua'
lua_srcs = ['lua/src/lapi.c', 'lua/src/lcode.c', 'lua/src/lctype.c', 'lua/src/ldebug.c', 'lua/src/ldo.c', 'lua/src/ldump.c', 'lua/src/lfunc.c', 'lua/src/lgc.c', 'lua/src/llex.c', 'lua/src/lmem.c', 'lua/src/lobject.c', 'lua/src/lopcodes.c', 'lua/src/lparser.c', 'lua/src/lstate.c', 'lua/src/lstring.c', 'lua/src/ltable.c', 'lua/src/ltm.c', 'lua/src/lundump.c', 'lua/src/lvm.c', 'lua/src/lzio.c', 'lua/src/lauxlib.c', 'lua/src/lbaselib.c', 'lua/src/lcorolib.c', 'lua/src/ldblib.c', 'lua/src/liolib.c', 'lua/src/lmathlib.c', 'lua/src/loadlib.c', 'lua/src/loslib.c', 'lua/src/lstrlib.c', 'lua/src/ltablib.c', 'lua/src/lutf8lib.c', 'lua/src/linit.c']
lua_lib = static_library(lua, lua_srcs,
    include_directories: 'lua/src/'
)

'''
zlib = 'z' # required by libi2pd
zlib_srcs = ['zlib/adler32.c', 'zlib/compress.c', 'zlib/crc32.c', 'zlib/deflate.c', 'zlib/gzclose.c', 'zlib/gzlib.c', 'zlib/gzread.c', 'zlib/gzwrite.c', 'zlib/infback.c', 'zlib/inffast.c', 'zlib/inflate.c', 'zlib/inftrees.c', 'zlib/trees.c', 'zlib/uncompr.c', 'zlib/zutil.c']
zlib_lib = static_library(zlib, zlib_srcs,
    include_directories: 'zlib/',
    link_args: '-Wno-implicit-function-declaration' # not working!!!
)

i2pd = 'i2pd'
i2pd_srcs = ['i2pd/libi2pd/api.cpp', 'i2pd/libi2pd/Gost.cpp', 'i2pd/libi2pd/RouterInfo.cpp', 'i2pd/libi2pd/Base.cpp', 'i2pd/libi2pd/Gzip.cpp', 'i2pd/libi2pd/Signature.cpp', 'i2pd/libi2pd/Blinding.cpp', 'i2pd/libi2pd/HTTP.cpp', 'i2pd/libi2pd/SSU2.cpp', 'i2pd/libi2pd/I2NPProtocol.cpp', 'i2pd/libi2pd/SSU2Session.cpp', 'i2pd/libi2pd/Config.cpp', 'i2pd/libi2pd/I2PEndian.cpp', 'i2pd/libi2pd/Streaming.cpp', 'i2pd/libi2pd/Identity.cpp', 'i2pd/libi2pd/Timestamp.cpp', 'i2pd/libi2pd/Crypto.cpp', 'i2pd/libi2pd/KadDHT.cpp', 'i2pd/libi2pd/TransitTunnel.cpp', 'i2pd/libi2pd/CryptoKey.cpp', 'i2pd/libi2pd/LeaseSet.cpp', 'i2pd/libi2pd/Transports.cpp', 'i2pd/libi2pd/Datagram.cpp', 'i2pd/libi2pd/Log.cpp', 'i2pd/libi2pd/TunnelConfig.cpp', 'i2pd/libi2pd/Destination.cpp', 'i2pd/libi2pd/NetDb.cpp', 'i2pd/libi2pd/Tunnel.cpp', 'i2pd/libi2pd/ECIESX25519AEADRatchetSession.cpp', 'i2pd/libi2pd/NetDbRequests.cpp', 'i2pd/libi2pd/TunnelEndpoint.cpp', 'i2pd/libi2pd/Ed25519.cpp', 'i2pd/libi2pd/NTCP2.cpp', 'i2pd/libi2pd/TunnelGateway.cpp', 'i2pd/libi2pd/Elligator.cpp', 'i2pd/libi2pd/TunnelPool.cpp', 'i2pd/libi2pd/Family.cpp', 'i2pd/libi2pd/Profiling.cpp', 'i2pd/libi2pd/util.cpp', 'i2pd/libi2pd/FS.cpp', 'i2pd/libi2pd/Reseed.cpp', 'i2pd/libi2pd/Garlic.cpp', 'i2pd/libi2pd/RouterContext.cpp', 'i2pd/libi2pd/SSU2OutOfSession.cpp', 'i2pd/libi2pd/TunnelBase.cpp', 'i2pd/libi2pd/PostQuantum.cpp'] # 'i2pd/libi2pd/*.cpp'
i2pd_lib = static_library(i2pd, i2pd_srcs,
    include_directories: 'i2pd/libi2pd/',
    cpp_args: '-DOPENSSL_SUPPRESS_DEPRECATED'
)

i2pd_client = 'i2pdclient'
i2pd_client_srcs = ['i2pd/libi2pd_client/AddressBook.cpp', 'i2pd/libi2pd_client/HTTPProxy.cpp', 'i2pd/libi2pd_client/I2PTunnel.cpp', 'i2pd/libi2pd_client/SOCKS.cpp', 'i2pd/libi2pd_client/BOB.cpp', 'i2pd/libi2pd_client/I2CP.cpp', 'i2pd/libi2pd_client/MatchedDestination.cpp', 'i2pd/libi2pd_client/UDPTunnel.cpp', 'i2pd/libi2pd_client/ClientContext.cpp', 'i2pd/libi2pd_client/I2PService.cpp', 'i2pd/libi2pd_client/SAM.cpp'] # 'i2pd/libi2pd_client/*.cpp'
i2pd_client_lib = static_library(i2pd_client, i2pd_client_srcs,
    include_directories: ['i2pd/i18n/', 'i2pd/libi2pd/', 'i2pd/libi2pd_client/'],
    cpp_args: '-DOPENSSL_SUPPRESS_DEPRECATED'
)

i2pd_lang = 'i2pdlang'
i2pd_lang_srcs = ['i2pd/i18n/Afrikaans.cpp', 'i2pd/i18n/English.cpp', 'i2pd/i18n/Italian.cpp', 'i2pd/i18n/Spanish.cpp', 'i2pd/i18n/Ukrainian.cpp', 'i2pd/i18n/Armenian.cpp', 'i2pd/i18n/French.cpp', 'i2pd/i18n/Polish.cpp', 'i2pd/i18n/Swedish.cpp', 'i2pd/i18n/Uzbek.cpp', 'i2pd/i18n/Chinese.cpp', 'i2pd/i18n/German.cpp', 'i2pd/i18n/Portuguese.cpp', 'i2pd/i18n/Turkish.cpp', 'i2pd/i18n/Czech.cpp', 'i2pd/i18n/I18N.cpp', 'i2pd/i18n/Russian.cpp', 'i2pd/i18n/Turkmen.cpp'] # 'i2pd/i18n/*.cpp'
i2pd_lang_lib = static_library(i2pd_lang, i2pd_lang_srcs,
    include_directories: ['i2pd/i18n/', 'i2pd/libi2pd/', 'i2pd/libi2pd_client/'],
    cpp_args: '-DOPENSSL_SUPPRESS_DEPRECATED'
)

i2pd_daemon = 'i2pddaemon'
i2pd_daemon_srcs = ['i2pd/daemon/Daemon.cpp', 'i2pd/daemon/HTTPServer.cpp', 'i2pd/daemon/I2PControl.cpp', 'i2pd/daemon/I2PControlHandlers.cpp', 'i2pd/daemon/UPnP.cpp']
i2pd_daemon_cpp_args = []
if host_machine.system() == 'linux' or host_machine.system() == 'darwin' or host_machine.system() == 'freebsd'
    i2pd_daemon_srcs += ['i2pd/daemon/UnixDaemon.cpp']
endif
if host_machine.system() == 'windows'
    i2pd_daemon_srcs += ['i2pd/Win32/DaemonWin32.cpp', 'i2pd/Win32/Win32App.cpp', 'i2pd/Win32/Win32Service.cpp', 'i2pd/Win32/Win32NetState.cpp']
    i2pd_daemon_cpp_args += ['-DWIN32_APP', '-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX']
endif
i2pd_daemon_lib = static_library(i2pd_daemon, i2pd_daemon_srcs,
    include_directories: ['i2pd/daemon/', 'i2pd/i18n/', 'i2pd/libi2pd/', 'i2pd/libi2pd_client/'],
    cpp_args: i2pd_daemon_cpp_args + ['-DOPENSSL_SUPPRESS_DEPRECATED']
)
'''

fmt = 'fmt'
fmt_srcs = ['fmt/src/format.cc', 'fmt/src/os.cc']
fmt_lib = static_library(fmt, fmt_srcs,
    include_directories: 'fmt/include/'
)

# meson setup --reconfigure ../build
# cd ../build
# meson compile
