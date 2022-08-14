workspace("NeroshopWorkspace")
    configurations({ "Debug", "Release" })
    location("build") -- specifies location of Makefiles
    
--------------------------------------    
-- neroshop
project("neroshop")
    kind("ConsoleApp")--("WindowedApp")
    language("C++")
    cppdialect("C++17")
    -- todo: remove src/icon from CLI
    files({ "src/buyer.cpp", --[["src/carrier.cpp",]] "src/cart.cpp", "src/catalog.cpp", "src/client.cpp", "src/converter.cpp", "src/database.cpp", "src/encryptor.cpp", "src/icon.cpp", "src/item.cpp", "src/main.cpp", "src/message.cpp", "src/order.cpp", "src/process.cpp", "src/qr.cpp", "src/seller.cpp", "src/server.cpp", "src/user.cpp", "src/validator.cpp", "src/wallet.cpp", })
    includedirs ({ 
        "include/", "external/monero-cpp/src/", --[["external/libbcrypt/",]] "external/sqlite/", "external/QR-Code-generator/cpp/", "external/json/single_include/", --[["external/curl/include/", "external/curl/lib/",]] --[["/usr/include/postgresql/", "/usr/include/postgresql/server/",]] "external/raft/include/", --[["external/libuv/include/", "external/libuv/src/",]] "external/monero-cpp/external/monero-project/external/db_drivers/liblmdb/", "external/stduuid", "external/stduuid/catch", "external/stduuid/include", "external/linenoise",
        -- dokun-ui
        "external/dokun-ui/include/",
        "external/dokun-ui/external/lua/lua-5.4.1/src/",
        "external/dokun-ui/external/glew/include/",
        "external/dokun-ui/external/glm/",
        "external/dokun-ui/external/png/", "external/dokun-ui/external/zlib/",
        "external/dokun-ui/external/jpeg/jpeg-6b/",
        "external/dokun-ui/external/tiff/libtiff/", -- not really in use at the moment. We only need png and jpeg
        "external/dokun-ui/external/gif/lib/", -- not really in use at the moment. We only need png and jpeg
        "external/dokun-ui/external/freetype/include/", "external/dokun-ui/external/freetype/include/freetype/", "external/freetype/include/freetype/config/", "external/dokun-ui/external/freetype/include/freetype/internal/",
        -- monero
        "external/monero-cpp/external/monero-project/external/rapidjson/include",
        "external/monero-cpp/external/monero-project/contrib/epee/include",
        "external/monero-cpp/external/monero-project/external/easylogging++",
        "external/monero-cpp/external/monero-project/src",
        "external/monero-cpp/external/monero-project/external/",
    })
    libdirs({ "src/", "build/bin/Debug/", "build/bin/Release/", "external/dokun-ui/build/", "external/dokun-ui/src/", })
    links({ "monero-cpp", --[["bcrypt",]] "sqlite3", "qrcodegen", "curl", --[["crypto", "ssl",]] --[["pq",]] "raft", "uv", "linenoise",
        -- dokun-ui
        "dokun-ui", "glfw", --[["vulkan",]]
        -- if building without dokun-ui
        --"lua", "png", "z",
        -- monero
        "external/monero-cpp/external/monero-project/build/release/lib/wallet_merged",
        "external/monero-cpp/external/monero-project/build/release/lib/wallet",
        "external/monero-cpp/external/monero-project/build/release/src/crypto/wallet/wallet-crypto",
        "external/monero-cpp/external/monero-project/build/release/external/db_drivers/liblmdb/lmdb",
        "external/monero-cpp/external/monero-project/build/release/contrib/epee/src/epee",
        "external/monero-cpp/external/monero-project/build/release/external/unbound/unbound",
        "external/monero-cpp/external/monero-project/build/release/external/easylogging++/easylogging",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_core/cryptonote_core",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_basic/cryptonote_basic",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_basic/cryptonote_format_utils_basic",
        "external/monero-cpp/external/monero-project/build/release/src/mnemonics/mnemonics",
        "external/monero-cpp/external/monero-project/build/release/src/common/common",
        "external/monero-cpp/external/monero-project/build/release/src/crypto/cncrypto",
        "external/monero-cpp/external/monero-project/build/release/src/ringct/ringct",
        "external/monero-cpp/external/monero-project/build/release/src/ringct/ringct_basic",
        "external/monero-cpp/external/monero-project/build/release/src/blockchain_db/blockchain_db",
        "external/monero-cpp/external/monero-project/build/release/src/blocks/blocks",
        "external/monero-cpp/external/monero-project/build/release/src/checkpoints/checkpoints",
        "external/monero-cpp/external/monero-project/build/release/src/device/device",
        "external/monero-cpp/external/monero-project/build/release/src/device_trezor/device_trezor",
        "external/monero-cpp/external/monero-project/build/release/src/multisig/multisig",
        "external/monero-cpp/external/monero-project/build/release/src/version",
        "external/monero-cpp/external/monero-project/build/release/external/randomx/randomx",
        -- monero-depends
        "protobuf", "usb-1.0", "boost_chrono", "boost_date_time", "boost_filesystem", "boost_program_options", "boost_regex", "boost_serialization", "boost_wserialization", "boost_system", "boost_thread", --[["udev",]] "hidapi-libusb", "sodium", "crypto", "ssl",
    })
    defines({ --[["NEROSHOP_USE_LIBBCRYPT",]] --[["NEROSHOP_USE_POSTGRESQL",]] --[["UUID_SYSTEM_GENERATOR",]] })
    if os.host() == "linux" then -- same as os.get(), except os.get() is deprecated in premake5
        links({ "pthread", "dl", "X11", })
    end--if os.host() == "windows" then links {} end--if os.host() == "macosx" then links {} end--if os.host() == "android" then links {} end
    location("build") -- specifies location of project binaries
    filter({ "system:windows" })
 		links({ "OpenGL32" })
	filter ({ "system:not windows" })
		links({ "GL" })
	--[[filter({ "system:linux" })
	    links({ "uuid" })]]-- -- link to libuuid if UUID_SYSTEM_GENERATOR is defined
		
    filter({ "configurations:Debug" })
        defines({ "DEBUG", "NEROSHOP_DEBUG" })
        symbols("On")
    filter ({ "configurations:Release" })
        defines({ "NDEBUG" })
        optimize("On")   
      
--------------------------------------
-- neromon
project("neromon")
    kind("ConsoleApp")
    language("C++")
    cppdialect("C++17")
    files({ "src/daemon.cpp", "src/server.cpp", --[["src/database.cpp",]] })
    includedirs({ "include/", })
    links { "uv" }   
--------------------------------------   
-- premake5 --cc=gcc --os=linux gmake
-- cd build && make
-- ./bin/Debug/neroshop
