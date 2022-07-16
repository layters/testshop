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
