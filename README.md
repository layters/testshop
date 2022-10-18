# neroshop - WORK IN PROGRESS
[![banner](images/appicons/neroshop-logo.png)](https://github.com/larteyoh/testshop "neroshop logo")


A P2P (peer-to-peer) marketplace for [**Monero**](https://getmonero.org/) users


## Table of contents
<!-- - [The history behind neroshop](#about)-->
- [Demo](#demo) <!-- - [Features](#features)--> <!-- - [Documentation](#documentation)-->
- [Building neroshop](#building-neroshop)
  - [Dependencies](#dependencies)
  - [Compiling neroshop from source](#compiling-neroshop-from-source) <!-- - [Contributing](#contributing) --> <!-- - [Bug Bounty Program]-->
- [Contact information](#contact)


## Demo
<!-- place link to videos here -->

<details>
<summary>Click to load images</summary>
    
![Wallet_Keys_Generation](https://github.com/larteyoh/testshop/blob/main/images/screenshots/Wallet_Keys_Generation_2022-10-12_04-34-24.png)
![Registration](https://github.com/larteyoh/testshop/blob/main/images/screenshots/Registration_2022-10-17_22-40-12.png)
![CatalogGrid_Top](https://github.com/larteyoh/testshop/blob/main/images/screenshots/CatalogGrid_Top_022-09-26_04-08-42.png)
![CatalogGrid_Bottom](https://github.com/larteyoh/testshop/blob/main/images/screenshots/CatalogGrid_Bottom_2022-09-26_04-08-56.png)

</details>

<!--## About
*neroshop* is a distributed P2P (peer-to-peer) marketplace that uses [**Monero**](https://getmonero.org/) as its default cryptocurrency and 
caters not only to darknet market users, but also those who believe in a **truly** free market that is uncensorable, unseizable, and unregulatable.
Neroshop aims to be simple for a beginner to use and easy for sellers to onboard their shop with just a few clicks.


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
| [Qt](https://www.qt.io/)                                           | 5.12.8             | graphical user interface                                               | :heavy_check_mark:                  |
| [libuv](https://github.com/libuv/libuv)                            | ?                  | networking and child process                                           | :heavy_check_mark:                  |
| [raft](https://github.com/willemt/raft)                            | ?                  | consensus mechanism                                                    | :heavy_check_mark:                  |
| [stduuid](https://github.com/mariusbancila/stduuid)                | ?                  | order number generation                                                | :heavy_check_mark:                  |
| [linenoise](https://github.com/antirez/linenoise)                  | ?                  | command line interface                                                 | :heavy_check_mark: :o:              |
| [lua](https://www.lua.org/)                                        | 5.1.5              | configuration script                                                   | :heavy_check_mark:                  |
| [libpng](http://www.libpng.org/pub/png/)                           | 1.6.37             | image exportation                                                      | :heavy_check_mark:                  |
| [zlib](https://www.zlib.net/)                                      | 1.2.12             | for libpng                                                             | :heavy_check_mark:                  |
| [openpgp](external/openpgp)                                        | ?                  | public-key encryption and digital signatures                           | :grey_question:                     |

### Compiling neroshop from source
**0. Clone neroshop (and its submodules)**
```bash
git clone --recurse-submodules https://github.com/larteyoh/testshop.git
```
```bash
cd testshop
```


**1. Install dependencies**

Debian/Ubuntu
```bash
# prerequisites
sudo apt install build-essential cmake git
# neroshop
sudo apt install libcurl4-openssl-dev libssl-dev libuv1-dev qtdeclarative5-dev qml-module-qt-labs-platform qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qtquick-shapes
# monero-cpp (monero)
sudo apt update && sudo apt install pkg-config libssl-dev libzmq3-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libpgm-dev qttools5-dev-tools libhidapi-dev libusb-1.0-0-dev libprotobuf-dev protobuf-compiler libudev-dev libboost-chrono-dev libboost-date-time-dev libboost-filesystem-dev libboost-locale-dev libboost-program-options-dev libboost-regex-dev libboost-serialization-dev libboost-system-dev libboost-thread-dev python3 ccache
```
Arch (needs to be updated)
```bash
# prerequisites
sudo pacman -Sy --needed base-devel cmake git
# neroshop
sudo pacman -Sy --needed curl openssl libuv
# monero-cpp (monero)
sudo pacman -Syu --needed boost openssl zeromq libpgm libsodium libunwind xz readline gtest python3 ccache qt5-tools hidapi libusb protobuf systemd
```
Fedora (needs to be updated)
```bash
# prerequisites
sudo dnf install gcc gcc-c++ make cmake git
# neroshop
sudo dnf install libcurl-devel openssl-devel libuv-devel libuv-static
# monero-cpp (monero)
sudo dnf install boost-static libstdc++-static pkgconf boost-devel openssl-devel zeromq-devel openpgm-devel libsodium-devel libunwind-devel xz-devel readline-devel gtest-devel ccache qt5-linguist hidapi-devel libusbx-devel protobuf-devel protobuf-compiler systemd-devel
```


**2. Update monero-cpp submodules**
```bash
cd external/monero-cpp && ./bin/update_submodules.sh
```
```bash
cd external/monero-project
```


**3. Install expat (dependency of unbound) and unbound:**
```bash
wget https://github.com/libexpat/libexpat/releases/download/R_2_4_8/expat-2.4.8.tar.bz2
tar -xf expat-2.4.8.tar.bz2
rm expat-2.4.8.tar.bz2
cd expat-2.4.8
./configure --enable-static --disable-shared
make
sudo make install
cd ../
```

```bash
wget https://www.nlnetlabs.nl/downloads/unbound/unbound-1.16.1.tar.gz
tar -xzf unbound-1.16.1.tar.gz
rm unbound-1.16.1.tar.gz
cd unbound-1.16.1
./configure --disable-shared --enable-static --without-pyunbound --with-libevent=no --without-pythonmodule --disable-flto --with-pthreads --with-libunbound-only --with-pic
make
sudo make install
cd ../
```

For Fedora users, you may need to add this line under the "find_package(Boost .." in case of an "undefined reference to icu_*" error:
`set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -licuio -licui18n -licuuc -licudata")`

<!-- git submodule update --init --force --> <!-- <= call this before building monero -->

**4. Build monero-project to create .a libraries**
```bash
make release-static
```
```bash
cd ../../../../
```


**5. Build neroshop**

To build with [**CMake**](https://cmake.org/):

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


To build with [**Premake**](https://premake.github.io/) (experimental - broken for now):

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


To build for [**Android**](https://www.android.com/) (requires [**Android NDK**](https://developer.android.com/ndk)):<!-- and [CMake](https://cmake.org/)):-->
```bash
```


## Contact
> larteyoh@pm.me

[//]: # (./clean.sh)
[//]: # (git checkout -b main)
[//]: # (git add .gitignore .gitmodules cmake/ CMakeLists.txt external/ fonts/ images/ main.qml premake5.lua qml/ qml.qrc README.md src/ test/)
[//]: # (git commit -m"...")
[//]: # (git push -u origin main --force)
[//]: # (https://git.slipfox.xyz/larteyoh/testshop/settings => Mirror Settings => Synchronize Now)
[//]: # (removing an external lib from submodules index: git rm --cached path/to/submodule)
