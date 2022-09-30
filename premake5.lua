workspace("NeroshopWorkspace")
    configurations({ "Debug", "Release" })
    location("build") -- specifies location of Makefiles
    
--------------------------------------    
-- neroshop
project("neroshop")
    kind("WindowedApp")--("ConsoleApp")
    language("C++")
    cppdialect("C++17")
    files({ "src/buyer.cpp", --[["src/carrier.cpp",]] "src/cart.cpp", "src/catalog.cpp", "src/client.cpp", "src/config.cpp", "src/converter.cpp", "src/database.cpp", "src/encryptor.cpp", "src/item.cpp", "src/order.cpp", "src/process.cpp", "src/qr.cpp", "src/script.cpp", "src/seller.cpp", "src/server.cpp", "src/user.cpp", "src/validator.cpp", "src/wallet.cpp", 
        -- neroshop_gui_sources
        "src/gui/icon.cpp", "src/gui/main.cpp", "src/gui/main_window.cpp", "src/gui/message_box.cpp", "src/gui/wallet_proxy.cpp" })
    includedirs ({ 
        "src/", "external/monero-cpp/src/", --[["external/libbcrypt/",]] "external/sqlite/", "external/QR-Code-generator/cpp/", "external/json/single_include/", --[["external/curl/include/", "external/curl/lib/",]] --[["/usr/include/postgresql/", "/usr/include/postgresql/server/",]] "external/raft/include/", --[["external/libuv/include/", "external/libuv/src/",]] "external/monero-cpp/external/monero-project/external/db_drivers/liblmdb/", "external/stduuid", "external/stduuid/catch", "external/stduuid/include", "external/lua/src/", "external/png/", "external/zlib/", "external/linenoise",
        -- monero
        "external/monero-cpp/external/monero-project/external/rapidjson/include",
        "external/monero-cpp/external/monero-project/contrib/epee/include",
        "external/monero-cpp/external/monero-project/external/easylogging++",
        "external/monero-cpp/external/monero-project/src",
        "external/monero-cpp/external/monero-project/external/",
        -- qt5
        "/usr/include/x86_64-linux-gnu/qt5/", "/usr/include/x86_64-linux-gnu/qt5/QtCore/", "/usr/include/x86_64-linux-gnu/qt5/QtGui/", "/usr/include/x86_64-linux-gnu/qt5/QtQml/", "/usr/include/x86_64-linux-gnu/qt5/QtQuick/",
    })
    libdirs({ "src/", "build/bin/Debug/", "build/bin/Release/", })
    links({ "monero-cpp", --[["bcrypt",]] "sqlite3", "qrcodegen", "curl", --[["crypto", "ssl",]] --[["pq",]] "raft", "uv", "linenoise",
        "lua", "png", "z",
        -- monero
        "external/monero-cpp/external/monero-project/build/release/lib/wallet",
        "external/monero-cpp/external/monero-project/build/release/src/rpc/rpc_base",
        "external/monero-cpp/external/monero-project/build/release/src/net/net",
        "external/monero-cpp/external/monero-project/build/release/external/db_drivers/liblmdb/lmdb",
        "unbound",--"external/monero-cpp/external/monero-project/build/release/external/unbound/unbound", --"/usr/local/lib/unbound",
        "external/monero-cpp/external/monero-project/build/release/external/easylogging++/easylogging",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_core/cryptonote_core",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_protocol/cryptonote_protocol",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_basic/cryptonote_basic",
        "external/monero-cpp/external/monero-project/build/release/src/cryptonote_basic/cryptonote_format_utils_basic",
        "external/monero-cpp/external/monero-project/build/release/src/mnemonics/mnemonics",
        "external/monero-cpp/external/monero-project/build/release/src/ringct/ringct",
        "external/monero-cpp/external/monero-project/build/release/src/ringct/ringct_basic",
        "external/monero-cpp/external/monero-project/build/release/src/common/common",
        "external/monero-cpp/external/monero-project/build/release/src/crypto/cncrypto",        
        "external/monero-cpp/external/monero-project/build/release/src/blockchain_db/blockchain_db",
        "external/monero-cpp/external/monero-project/build/release/src/blocks/blocks",
        "external/monero-cpp/external/monero-project/build/release/src/checkpoints/checkpoints",
        "external/monero-cpp/external/monero-project/build/release/src/device/device",
        "external/monero-cpp/external/monero-project/build/release/src/device_trezor/device_trezor",
        "external/monero-cpp/external/monero-project/build/release/src/multisig/multisig",
        "external/monero-cpp/external/monero-project/build/release/src/version",
        "external/monero-cpp/external/monero-project/build/release/external/randomx/randomx",
        "external/monero-cpp/external/monero-project/build/release/contrib/epee/src/epee",
        "external/monero-cpp/external/monero-project/build/release/src/hardforks/hardforks",  
        --"external/monero-cpp/external/monero-project/build/release/src/crypto/cncrypto", -- monero-cpp links to this twice. Not sure why :/             
        "external/monero-cpp/external/monero-project/build/release/src/crypto/wallet/wallet-crypto",
        -- monero-depends
        "unbound", "boost_chrono", "boost_date_time", "boost_filesystem", "boost_program_options", "boost_regex", "boost_serialization", "boost_wserialization", "boost_system", "boost_thread", "protobuf", "usb-1.0", "crypto", "ssl", "sodium", --[["udev",]] "hidapi-libusb",
        -- Qt5
        "Qt5Core", "Qt5Qml", "Qt5Quick", "Qt5Gui",
    })
    defines({ "HAVE_HIDAPI", "NEROSHOP_BUILD_GUI", "NEROSHOP_USE_QT" --[["NEROSHOP_USE_LIBBCRYPT",]] --[["NEROSHOP_USE_POSTGRESQL",]] --[["UUID_SYSTEM_GENERATOR",]] })
    buildoptions { "-fPIC", }
    if os.host() == "linux" then -- same as os.get(), except os.get() is deprecated in premake5
        links({ "pthread", "dl", --[["X11",]] })
    end--if os.host() == "windows" then links {} end--if os.host() == "macosx" then links {} end--if os.host() == "android" then links {} end
    location("build") -- specifies location of project binaries
    filter({ "system:windows" })
 		links({ "OpenGL32" })
	filter ({ "system:not windows" })
		links({ "GL" })
    -- Link to libuuid if UUID_SYSTEM_GENERATOR is defined
	--[[filter({ "system:linux" })
	    links({ "uuid" })]]--
		
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
    files({ "src/daemon/daemon.cpp", "src/server.cpp", --[["src/database.cpp",]] })
    includedirs({ "src/", "external/monero-cpp/src/", --[["external/libbcrypt/",]] "external/sqlite/", "external/QR-Code-generator/cpp/", "external/json/single_include/", --[["external/curl/include/", "external/curl/lib/",]] --[["/usr/include/postgresql/", "/usr/include/postgresql/server/",]] "external/raft/include/", --[["external/libuv/include/", "external/libuv/src/",]] "external/monero-cpp/external/monero-project/external/db_drivers/liblmdb/", "external/stduuid", "external/stduuid/catch", "external/stduuid/include", "external/lua/src/", "external/png/", "external/zlib/", "external/linenoise", })
    links { "uv" }   
--------------------------------------   
-- premake5 --cc=gcc --os=linux gmake
-- cd build && make
-- ./bin/Debug/neroshop
