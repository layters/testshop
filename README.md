# neroshop 

[![alt text](res/neroshop-logo.png)](https://github.com/larteyoh/testshop "neroshop logo")

## Overview
A P2P marketplace for [Monero](https://getmonero.org/) users


## Table of contents
<!-- - [The history behind neroshop](#about)-->
<!-- - [Features](#features)--> <!-- - [Documentation](#documentation)-->
- [Building neroshop](#building-neroshop)
  - [Dependencies](#dependencies)
  - [Compiling neroshop from source](#compiling-neroshop-from-source) <!-- - [Contributing](#contributing) --> <!-- - [Bug Bounty Program]-->
- [Contact information](#contact)


<!--## About
This is currently a work in progress. There is nothing to see here :shrug:


## Features
Coming soon

-->
## Building neroshop

### Dependencies
:heavy_check_mark: = Currently in use; :o: = Optional; :x: = Not currently in use or removed; :grey_question: = Not yet in use, but up for consideration

|      Library                                                       | Minimum Ver.       |         Purpose                                                        | Status                              |
|--------------------------------------------------------------------|--------------------|------------------------------------------------------------------------|-------------------------------------|
| [monero-cpp](https://github.com/monero-ecosystem/monero-cpp)       | latest             | monero wallet and payment system                                       | :heavy_check_mark:                  |
| [libbcrypt](https://github.com/rg3/libbcrypt)                      | 1.3                | password hashing                                                       | :o:                                 |
| [sqlite3](https://sqlite.org/)                                     | 3.38.0             | database management                                                    | :heavy_check_mark:                  |
| [QR Code generator](https://github.com/nayuki/QR-Code-generator)   | ?                  | qr code generation                                                     | :heavy_check_mark:                  |
| [json](https://github.com/nlohmann/json/)                          | ?                  | json parsing                                                           | :heavy_check_mark:                  |
| [curl](https://github.com/curl/curl)                               | ?                  | currency conversion                                                    | :heavy_check_mark:                  |
| [openssl](https://github.com/openssl/openssl)                      | 1.1.1              | for curl, sha256 sum and message encryption                            | :heavy_check_mark:                  |
| [dokun-ui](external/dokun-ui)                                      | n/a                | graphical user interface                                               | :heavy_check_mark:                  |
| [libuv](https://github.com/libuv/libuv)                            | ?                  | networking, child process and file system                              | :heavy_check_mark:                  |
| [raft](https://github.com/willemt/raft)                            | ?                  | consensus mechanism                                                    | :heavy_check_mark:                  |
| [stduuid](https://github.com/mariusbancila/stduuid)                | ?                  | order number generation                                                | :heavy_check_mark:                  |
| [linenoise](https://github.com/antirez/linenoise)                  | ?                  | command line interface                                                 | :o:                                 |

### Compiling neroshop from source
**0. Clone neroshop**
```bash
git clone https://github.com/larteyoh/testshop.git && cd testshop
```


**1. Install dependencies**

Debian/Ubuntu
```bash
sudo -s -- << EOF
# prerequisites
sudo apt install build-essential cmake git
# neroshop, dokun-ui
sudo apt install libx11-dev libgl1-mesa-dev libglu1-mesa-dev libcurl4-openssl-dev libssl-dev libuv1-dev
# monero-cpp (monero)
sudo apt update && sudo apt install pkg-config libssl-dev libzmq3-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev libpgm-dev qttools5-dev-tools libhidapi-dev libusb-1.0-0-dev libprotobuf-dev protobuf-compiler libudev-dev libboost-chrono-dev libboost-date-time-dev libboost-filesystem-dev libboost-locale-dev libboost-program-options-dev libboost-regex-dev libboost-serialization-dev libboost-system-dev libboost-thread-dev python3 ccache doxygen graphviz
EOF
```
Arch
```bash
# prerequisites
sudo pacman -Sy --needed base-devel cmake git
# neroshop, dokun-ui
sudo pacman -Sy --needed libx11 lib32-mesa lib32-glu curl openssl libuv
# monero-cpp (monero)
sudo pacman -Syu --needed boost openssl zeromq libpgm unbound libsodium libunwind xz readline ldns expat gtest python3 ccache doxygen graphviz qt5-tools hidapi libusb protobuf systemd
```
Fedora
```bash
# prerequisites
sudo dnf install gcc gcc-c++ make cmake git
# neroshop, dokun-ui
sudo dnf install libX11-devel mesa-libGL-devel mesa-libGLU-devel libcurl-devel openssl-devel libuv-devel libuv-static
# monero-cpp (monero)
sudo dnf install boost-static libstdc++-static pkgconf boost-devel openssl-devel zeromq-devel openpgm-devel unbound-devel libsodium-devel libunwind-devel xz-devel readline-devel ldns-devel expat-devel gtest-devel ccache doxygen graphviz qt5-linguist hidapi-devel libusbx-devel protobuf-devel protobuf-compiler systemd-devel
```


**2. Clone submodules and nested submodules**
```bash
cd external
git clone --recurse-submodules https://github.com/monero-ecosystem/monero-cpp.git
#git clone --recurse-submodules https://github.com/rg3/libbcrypt.git
#git clone --recurse-submodules https://github.com/sqlite/sqlite.git
git clone --recurse-submodules https://github.com/nayuki/QR-Code-generator.git
git clone --recurse-submodules https://github.com/nlohmann/json.git
#git clone --recurse-submodules https://github.com/curl/curl.git
#git clone --recurse-submodules https://github.com/libuv/libuv.git
git clone --recurse-submodules https://github.com/willemt/raft.git
git clone --recurse-submodules https://github.com/mariusbancila/stduuid.git
git clone https://github.com/antirez/linenoise.git
cd ../
```


**3. Modify external/monero-cpp/external/monero-project/CMakeLists.txt:**
`option(BUILD_GUI_DEPS "Build GUI dependencies." ON)`

For Fedora users, you may need to add this line under the "find_package(Boost .." in case of an "undefined reference to icu_*" error:
`set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -licuio -licui18n -licuuc -licudata")`


**4. Build monero-project twice to create libwallet_merged.a and other .a libraries**
```bash
cd external/monero-cpp/external/monero-project && make release-static && make release-static
cd ../../../../
```


**5. Build dokun-ui**
```bash
# Build dokun-ui
cd external/dokun-ui
mkdir build && cd build
cmake ..
make libdokun-ui -j4
cd ../../../
```


**6. Build neroshop**

To build with [CMake](https://cmake.org/):

```bash
# Build external libraries
cd external/
cmake -G"Unix Makefiles"
make
cd ..

# Build neroshop
cd build
cmake ..
make

# Run neroshop
./neroshop
```


To build with [Premake](https://premake.github.io/) (experimental):

```bash
# Build external libraries
cd external/
premake5 gmake
cd ../build && make

# Build neroshop
cd ..
premake5 gmake
cd build && make

# Run neroshop
./bin/Debug/neroshop
```


To build for [Android](https://www.android.com/) (requires [Android NDK](https://developer.android.com/ndk)):<!-- and [CMake](https://cmake.org/)):-->
```bash
```


## Contact
> larteyoh@pm.me

[//]: # (./clean.sh)
[//]: # (git checkout -b test)
[//]: # (git add .gitignore .gitmodules cmake/ CMakeLists.txt external/ premake5.lua README.md res/neroshop-logo.png res/wallets src/ test/)
[//]: # (git commit -m"...")
[//]: # (git push -u origin test)
[//]: # (https://git.slipfox.xyz/larteyoh/testshop/settings => Mirror Settings => Synchronize Now)
