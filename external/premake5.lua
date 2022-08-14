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
    
project "bcrypt"
    kind "StaticLib"
    language "C"
    files { 
        "libbcrypt/crypt_blowfish/crypt_blowfish.c", "libbcrypt/crypt_blowfish/crypt_gensalt.c", "libbcrypt/crypt_blowfish/wrapper.c", "libbcrypt/bcrypt.c",
    }
    includedirs { 
        "libbcrypt/"
    }

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
    
project "raft"
    kind "StaticLib"
    language "C"
    files { 
        "raft/src/raft_log.c", "raft/src/raft_node.c", "raft/src/raft_server.c", "raft/src/raft_server_properties.c",
    }
    includedirs { 
        "raft/include/"
    }

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

project "png"
    kind "StaticLib"
    language "C"
    files { 
        "png/png.c", "png/pngerror.c", "png/pngget.c", "png/pngmem.c", "png/pngpread.c", "png/pngread.c", "png/pngrio.c", "png/pngrtran.c", "png/pngrutil.c", "png/pngset.c", --[["png/pngtest.c",]] "png/pngtrans.c", "png/pngwio.c", "png/pngwrite.c", "png/pngwtran.c", "png/pngwutil.c",
    }
    includedirs { 
        "png/",
    }
    
project "z" --zlib
    kind "StaticLib"
    language "C"
    files { 
        "zlib/adler32.c", "zlib/compress.c", "zlib/crc32.c", "zlib/deflate.c", "zlib/gzclose.c", "zlib/gzlib.c", "zlib/gzread.c", "zlib/gzwrite.c", "zlib/infback.c", "zlib/inffast.c", "zlib/inflate.c", "zlib/inftrees.c", "zlib/trees.c", "zlib/uncompr.c", "zlib/zutil.c",
    }
    includedirs { 
        "zlib/",
    }
            
--[[
project "<library-name>"
    kind "StaticLib"
    language "C++"
    files { 
        
    }
    includedirs { 
        
    }
    --defines {}
]]  
   location "../build" -- specifies location of binaries
   filter { "configurations:Debug" }
      defines { "DEBUG" }
      symbols "On"

   filter { "configurations:Release" }
      defines { "NDEBUG" }
      optimize "On"     
      
-- premake5 --cc=gcc --os=linux gmake
-- cd ../build && make
