workspace "NeroshopExternalWorkspace"
    configurations { "Debug", "Release" }
    location "../build" -- specifies location of Makefiles
    
project "monero-cpp"
    kind "StaticLib"
    language "C++"
    files { 
        "monero-cpp/src/utils/gen_utils.cpp", "monero-cpp/src/utils/monero_utils.cpp", "monero-cpp/src/daemon/monero_daemon_model.cpp", "monero-cpp/src/daemon/monero_daemon.cpp", "monero-cpp/src/wallet/monero_wallet_model.cpp", "monero-cpp/src/wallet/monero_wallet_keys.cpp", "monero-cpp/src/wallet/monero_wallet_full.cpp",
    }
    includedirs { 
        "monero-cpp/src/", 
        "monero-cpp/external/monero-project/contrib/epee/include/", 
        "monero-cpp/external/monero-project/external/easylogging++/", 
        "monero-cpp/external/monero-project/src/", 
        "monero-cpp/external/monero-project/external/rapidjson/include/", 
        "monero-cpp/external/monero-project/external/",       
    }
    defines { "BOOST_DISABLE_PRAGMA_MESSAGE" }
    
project "qrcodegen"
    kind "StaticLib"
    language "C++"
    files { 
        "QR-Code-generator/cpp/qrcodegen.cpp"
    }
    includedirs { 
        "QR-Code-generator/cpp/"
    }
    
project "sqlite3"
    kind "StaticLib"
    language "C"
    files { 
        "sqlite/sqlite3.c"
    }
    includedirs { 
        "sqlite/"
    }
    defines { "SQLITE_ENABLE_FTS5" }
    
project "linenoise"
    kind "StaticLib"
    language "C"
    files { 
        "linenoise/linenoise.c",
    }
    includedirs { 
        "linenoise/",
    }

project "lua"
    kind "StaticLib"
    language "C"
    files { 
        "lua/src/lapi.c", "lua/src/lcode.c", "lua/src/lctype.c", "lua/src/ldebug.c", "lua/src/ldo.c", "lua/src/ldump.c", "lua/src/lfunc.c", "lua/src/lgc.c", "lua/src/llex.c", "lua/src/lmem.c", "lua/src/lobject.c", "lua/src/lopcodes.c", "lua/src/lparser.c", "lua/src/lstate.c", "lua/src/lstring.c", "lua/src/ltable.c", "lua/src/ltm.c", "lua/src/lundump.c", "lua/src/lvm.c", "lua/src/lzio.c", "lua/src/lauxlib.c", "lua/src/lbaselib.c", "lua/src/lcorolib.c", "lua/src/ldblib.c", "lua/src/liolib.c", "lua/src/lmathlib.c", "lua/src/loadlib.c", "lua/src/loslib.c", "lua/src/lstrlib.c", "lua/src/ltablib.c", "lua/src/lutf8lib.c", "lua/src/linit.c",
    }
    includedirs { 
        "lua/src/",
    }    

project "z" -- required by libi2pd
    kind "StaticLib"
    language "C"
    files { 
        "zlib/adler32.c", "zlib/compress.c", "zlib/crc32.c", "zlib/deflate.c", "zlib/gzclose.c", "zlib/gzlib.c", "zlib/gzread.c", "zlib/gzwrite.c", "zlib/infback.c", "zlib/inffast.c", "zlib/inflate.c", "zlib/inftrees.c", "zlib/trees.c", "zlib/uncompr.c", "zlib/zutil.c",
    }
    includedirs { 
        "zlib/",
    }
    buildoptions { "-Wno-implicit-function-declaration" }

project "i2pd"
    kind "StaticLib"
    language "C++"
    files { 
        "i2pd/libi2pd/**.cpp"
    }
    includedirs { 
        "i2pd/libi2pd/"
    }
    defines { "OPENSSL_SUPPRESS_DEPRECATED" }
    
project "i2pdclient"
    kind "StaticLib"
    language "C++"
    files { 
        "i2pd/libi2pd_client/**.cpp"
    }
    includedirs { 
        "i2pd/i18n/", "i2pd/libi2pd/", "i2pd/libi2pd_client/"
    }
    defines { "OPENSSL_SUPPRESS_DEPRECATED" }
    
project "i2pdlang"
    kind "StaticLib"
    language "C++"
    files { 
        "i2pd/i18n/**.cpp"
    }
    includedirs { 
        "i2pd/i18n/", "i2pd/libi2pd/", "i2pd/libi2pd_client/"
    }
    defines { "OPENSSL_SUPPRESS_DEPRECATED" }
    
project "i2pddaemon"
    kind "StaticLib"
    language "C++"
    files { 
        "i2pd/daemon/Daemon.cpp", "i2pd/daemon/HTTPServer.cpp", "i2pd/daemon/I2PControl.cpp", "i2pd/daemon/I2PControlHandlers.cpp", --[["i2pd/daemon/i2pd.cpp",]] "i2pd/daemon/UPnP.cpp"
    }
    includedirs { 
        "i2pd/daemon/", "i2pd/i18n/", "i2pd/libi2pd/", "i2pd/libi2pd_client/"
    }
    defines { "OPENSSL_SUPPRESS_DEPRECATED" }
    filter { "system:linux or system:macosx" }
        files { "i2pd/daemon/UnixDaemon.cpp" }
    filter { "system:windows" }
        files { "i2pd/Win32/DaemonWin32.cpp", "i2pd/Win32/Win32App.cpp", "i2pd/Win32/Win32Service.cpp", "i2pd/Win32/Win32NetState.cpp" }
        defines { "WIN32_APP", "WIN32_LEAN_AND_MEAN", "NOMINMAX" }

    location "../build" -- specifies location of binaries
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

-- premake5 --cc=gcc --os=linux gmake
-- cd ../build && make
