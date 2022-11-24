# neroshop - WORK IN PROGRESS
[![banner](images/appicons/LogoLight250x250.png)](https://github.com/larteyoh/testshop "neroshop logo")


A distributed P2P (peer-to-peer) marketplace for [**Monero**](https://getmonero.org/) users


## Table of contents
<!-- - [The history behind neroshop](#about)-->
- [Demo](#demo) <!-- - [Features](#features)--> <!-- - [Documentation](#documentation)-->
- [Building neroshop](#building-neroshop)
  - [Dependencies](#dependencies)
  - [Compiling neroshop from source](#compiling-neroshop-from-source) <!-- - [Contributing](#contributing) --> <!-- - [Bug Bounty Program]-->
- [Donations](#donations)
- [Resources](#resources)
- [Credits](#credits)


## Demo
<!-- place link to videos here -->

<details>
<summary>Click to load images</summary>
    
![Wallet_Keys_Generation](https://github.com/larteyoh/testshop/blob/main/images/screenshots/Wallet_Keys_Generation.png)
![Registration](https://github.com/larteyoh/testshop/blob/main/images/screenshots/Registration.png)
![CatalogGrid_Top](https://github.com/larteyoh/testshop/blob/main/images/screenshots/CatalogGrid_Top.png)
![CatalogGrid_Bottom](https://github.com/larteyoh/testshop/blob/main/images/screenshots/CatalogGrid_Bottom.png)

</details>

<!--## About
*neroshop* is a distributed P2P (peer-to-peer) marketplace that uses [**Monero**](https://getmonero.org/) as its default cryptocurrency and 
caters not only to darknet market users, but also those who believe in a **truly** free market that is uncensorable, unseizable, and unregulatable.
Neroshop aims to be simple for a beginner to use and easy for sellers to onboard their shop with just a few clicks.


## Planned Features
* Distributed P2P network
* Buy and sell products with Monero
* Anonymous payments
* No censorship (censorship-resistant)
* Pseudonymous identities (sellers and buyers are identified by their unique ids)
* No registration required (for buyers only)
* No KYC nor AML
* No listing fees, sales tax, or any other fees (except for miner transaction fees and shipping costs)
* End-to-end encrypted messaging system for communications between sellers and buyers
* Subaddress generator (a unique subaddresses will be generated from a seller's synced wallet account for each order placed by a customer)
* Option to run a local Monero node or connect to remote Monero nodes (so that sellers will not have to sync the entire blockchain)
* Option to choose between sending funds directly to a seller or using a multisignature escrow.
* Tor integration (Internet traffic can be optionally routed through tor for more added privacy)
* Reputation system
* Product rating system
* Wishlists


-->
## Building neroshop

### Dependencies
:heavy_check_mark: = Currently in use; :o: = Optional; :x: = Not currently in use or removed; :grey_question: = Not yet in use, but up for consideration; :white_square_button: = Exclusive to CLI

|      Library                                                       | Minimum Ver.       |         Purpose                                                        | Status                                   |
|--------------------------------------------------------------------|--------------------|------------------------------------------------------------------------|------------------------------------------|
| [monero-cpp](https://github.com/monero-ecosystem/monero-cpp)       | latest             | monero wallet and payment system                                       | :heavy_check_mark:                       |
| [libbcrypt](https://github.com/rg3/libbcrypt)                      | 1.3                | password hashing                                                       | :x:                                      |
| [sqlite3](https://sqlite.org/)                                     | 3.38.0             | database management                                                    | :heavy_check_mark:                       |
| [QR Code generator](https://github.com/nayuki/QR-Code-generator)   | ?                  | qr code generation                                                     | :heavy_check_mark:                       |
| [json](https://github.com/nlohmann/json/)                          | ?                  | json parsing                                                           | :o:                                      |
| [curl](https://github.com/curl/curl)                               | ?                  | currency conversion                                                    | :o:                                      |
| [openssl](https://github.com/openssl/openssl)                      | 1.1.1              | for curl, sha256 sum and message encryption                            | :heavy_check_mark:                       |
| [Qt](https://www.qt.io/)                                           | 5.12.8             | graphical user interface                                               | :heavy_check_mark:                       |
| [libuv](https://github.com/libuv/libuv)                            | ?                  | networking and child process                                           | :heavy_check_mark:                       |
| [raft](https://github.com/willemt/raft)                            | ?                  | consensus mechanism                                                    | :heavy_check_mark:                       |
| [stduuid](https://github.com/mariusbancila/stduuid)                | ?                  | order number generation                                                | :o:                                      |
| [linenoise](https://github.com/antirez/linenoise)                  | ?                  | command line interface                                                 | :heavy_check_mark: :white_square_button: |
| [lua](https://www.lua.org/)                                        | 5.1.5              | configuration script                                                   | :heavy_check_mark:                       |
| [libpng](http://www.libpng.org/pub/png/)                           | 1.6.37             | image exportation                                                      | :heavy_check_mark:                       |
| [zlib](https://www.zlib.net/)                                      | 1.2.12             | for libpng                                                             | :heavy_check_mark:                       |
| [openpgp](external/openpgp)                                        | ?                  | public-key encryption and digital signatures                           | :grey_question:                          |

### Compiling neroshop from source
**0. Install prerequisites**

Debian/Ubuntu
```bash
sudo apt install build-essential cmake git
```
Arch
```bash
sudo pacman -Sy --needed base-devel cmake git
```
Fedora
```bash
sudo dnf install gcc gcc-c++ make cmake git
```


**1. Clone neroshop (and its submodules)**
```bash
git clone --recurse-submodules https://github.com/larteyoh/testshop.git
```
```bash
cd testshop
```

**2. Install dependencies**

Debian/Ubuntu
```bash
# neroshop
sudo apt install libcurl4-openssl-dev libssl-dev libuv1-dev qtdeclarative5-dev qml-module-qt-labs-platform qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qtquick-shapes qml-module-qtquick-dialogs
# monero-cpp (monero)
sudo apt update && sudo apt install pkg-config libssl-dev libzmq3-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libpgm-dev qttools5-dev-tools libhidapi-dev libusb-1.0-0-dev libprotobuf-dev protobuf-compiler libudev-dev libboost-chrono-dev libboost-date-time-dev libboost-filesystem-dev libboost-locale-dev libboost-program-options-dev libboost-regex-dev libboost-serialization-dev libboost-system-dev libboost-thread-dev python3 ccache
```
Arch (needs to be updated)
```bash
# neroshop
sudo pacman -Sy --needed curl openssl libuv
# monero-cpp (monero)
sudo pacman -Syu --needed boost openssl zeromq libpgm unbound libsodium libunwind xz readline gtest python3 ccache qt5-tools hidapi libusb protobuf systemd
```
Fedora (needs to be updated)
```bash
# neroshop
sudo dnf install libcurl-devel openssl-devel libuv-devel libuv-static
# monero-cpp (monero)
sudo dnf install boost-static libstdc++-static pkgconf boost-devel openssl-devel zeromq-devel openpgm-devel unbound-devel libsodium-devel libunwind-devel xz-devel readline-devel gtest-devel ccache qt5-linguist hidapi-devel libusbx-devel protobuf-devel protobuf-compiler systemd-devel
```


**3. Update monero-cpp submodules**
```bash
cd external/monero-cpp && ./bin/update_submodules.sh
```
```bash
cd external/monero-project
```


**4. Install expat and unbound (May be required to build monero-project on Debian/Ubuntu otherwise, this step can be skipped):**
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
cd ../
```

In some cases, you may need to add this line under the "find_package(Boost .." in "external/monero-cpp/external/monero-project/CMakeLists.txt" in case of an "undefined reference to icu_*" error:
`set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -licuio -licui18n -licuuc -licudata")`

<!-- git submodule update --init --force --> <!-- <= call this before building monero -->

**5. Build monero-project to create .a libraries**
```bash
make release-static
```
```bash
cd ../../../../
```


**6. Build neroshop**

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


To build for [**Android**](https://www.android.com/) (requires [**Android NDK**](https://developer.android.com/ndk)):<!-- and [CMake](https://cmake.org/)):-->
```bash
```


## Donations
You may support the neroshop project directly by donating Monero (XMR) to the address below. Received payments will be used to reward developers for their contributions to the project (mostly by completing bounties) and will also be used to fund our official website domain name.
```
83QbQvnnyo7515rEnW8XwF1hbP5qMab6sHXFzP6pg3EKGscgXCbVjbt1FX5SF7AV9p4Ur1tiommuQSzrQQRHkZicVYu6j8Y
```
<p align="center">
    <a href="monero:83QbQvnnyo7515rEnW8XwF1hbP5qMab6sHXFzP6pg3EKGscgXCbVjbt1FX5SF7AV9p4Ur1tiommuQSzrQQRHkZicVYu6j8Y" target="_blank"><img src="images/donate_xmr.png" width="128" height="128"></img></a>
</p>


## Resources
> Website: [neroshop.org](https://neroshop.org/)

> Git (Unofficial): [github.com/larteyoh/testshop](https://github.com/larteyoh/testshop)

> Git (Official): [github.com/larteyoh/neroshop](https://github.com/larteyoh/neroshop)

> Mail: neroshop@protonmail.com

> Matrix: [#neroshop:matrix.org](https://matrix.to/#/#neroshop:matrix.org)


## Credits
Official neroshop logo design — [u/EchoingCat](https://www.reddit.com/u/EchoingCat)


[//]: # (./clean.sh)
[//]: # (git checkout -b main)
[//]: # (git add .gitignore .gitmodules cmake/ CMakeLists.txt external/ fonts/ images/ qml/ qml.qrc README.md shaders/ src/ test/)
[//]: # (git commit -m"...")
[//]: # (git push -u origin main --force)
[//]: # (https://git.slipfox.xyz/larteyoh/testshop/settings => Mirror Settings => Synchronize Now)
[//]: # (removing an external lib from submodules index: git rm --cached path/to/submodule)
