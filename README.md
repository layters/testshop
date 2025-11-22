# neroshop - WORK IN PROGRESS
[![banner](assets/images/appicons/LogoLight250x250.png)](https://github.com/layters/testshop "neroshop logo")


NeroShop is a decentralized peer-to-peer marketplace for trading goods and services with [**Monero**](https://getmonero.org/)


> [!Important]
> The neroshop team operates independently
> and is not affiliated, associated, authorized, endorsed by, or in any way officially connected
> with the Monero project, Monero team or any organization.


## Table of Contents
<!-- - [The history behind neroshop](#about)-->
- [Demo](#demo) <!-- - [Documentation](#documentation)-->
- [Building neroshop](#building-neroshop)
  - [Dependencies](#dependencies)
  - [Compiling neroshop from source](#compiling-neroshop-from-source)
  - [Setting up i2p](#setting-up-i2p)
- [Contributing](#contributing) <!-- - [Bug Bounty Program]-->
- [License](#license)
- [Donations](#donations)
- [Resources](#resources)
- [Thanks](#thanks)


## Demo
<details>
<summary>Click to load images</summary>
    
![Wallet_Keys_Generation](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Wallet_Keys_Generation.png)
![Registration](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Registration.png)
![Login](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Login.png)
![CatalogGrid](https://github.com/layters/testshop/blob/main/assets/images/screenshots/CatalogGrid.png)
![ProductPage](https://github.com/layters/testshop/blob/main/assets/images/screenshots/ProductPage.png)
![SettingsDialog_Network](https://github.com/layters/testshop/blob/main/assets/images/screenshots/SettingsDialog_Monero.png)
![SettingsDialog_General](https://github.com/layters/testshop/blob/main/assets/images/screenshots/SettingsDialog_General.png)
![SettingsDialog_Peers](https://github.com/layters/testshop/blob/main/assets/images/screenshots/SettingsDialog_Peers.png)
![HomePage](https://github.com/layters/testshop/blob/main/assets/images/screenshots/HomePage.png)
![HomePage_Recent_Listings](https://github.com/layters/testshop/blob/main/assets/images/screenshots/HomePage_Recent_Listings.png)
![Dashboard](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Dashboard_Overview.png)
![Store_Inventory](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Store_InventoryTab.png)
![Store_Inventory_ProductDialog_Top](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Store_InventoryTab_ProductDialog_Top.png)
![Store_Inventory_ProductDialog_Mid](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Store_InventoryTab_ProductDialog_Mid.png)
![Store_Inventory_ProductDialog_Bottom](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Store_InventoryTab_ProductDialog_Bottom.png)
![WalletPage_Send](https://github.com/layters/testshop/blob/main/assets/images/screenshots/WalletPage_BalanceSend.png)
![WalletPage_Receive](https://github.com/layters/testshop/blob/main/assets/images/screenshots/WalletPage_BalanceReceive.png)
![WalletPage_Transactions](https://github.com/layters/testshop/blob/main/assets/images/screenshots/WalletPage_BalanceTxs.png)
![ProfilePage_ListingsTab](https://github.com/layters/testshop/blob/main/assets/images/screenshots/ProfilePage_ListingsTab.png)
![ProfilePage_RatingsTab](https://github.com/layters/testshop/blob/main/assets/images/screenshots/ProfilePage_RatingsTab.png)
![ProfilePage_With_Custom_Avatar](https://github.com/layters/testshop/blob/main/assets/images/screenshots/ProfilePage_With_Custom_Avatar.png)
![MessagesPage](https://github.com/layters/testshop/blob/main/assets/images/screenshots/Messages_Page.png)

</details>


## Building neroshop

### Dependencies
:heavy_check_mark: = Currently in use | :o: = Optional | :x: = Marked for deprecation | :grey_question: = Not in use, but may be considered 

:white_square_button: = For CLI only | :package: = Bundled

|      Library                                                       | Minimum Ver.       |         Purpose                                                        | Status                                             |
|--------------------------------------------------------------------|--------------------|------------------------------------------------------------------------|----------------------------------------------------|
| [monero-cpp](https://github.com/woodser/monero-cpp)                | latest             | monero wallet and payment system                                       | :heavy_check_mark: :package:                       |
| [sqlite3](https://sqlite.org/)                                     | 3.38.0             | database management                                                    | :heavy_check_mark: :package:                       |
| [QR Code generator](https://github.com/nayuki/QR-Code-generator)   | ?                  | qr code generation                                                     | :heavy_check_mark: :package:                       |
| [json](https://github.com/nlohmann/json/)                          | ?                  | json parsing and msgpack                                               | :x: :package:                                      |
| [curl](https://github.com/curl/curl)                               | ?                  | currency conversion                                                    | :heavy_check_mark: :white_square_button:           |
| [openssl](https://github.com/openssl/openssl)                      | 1.1.1              | for curl, sha256 sum and message encryption                            | :heavy_check_mark:                                 |
| [Qt](https://www.qt.io/)                                           | 5.15.0             | graphical user interface                                               | :heavy_check_mark:                                 |
| [stduuid](https://github.com/mariusbancila/stduuid)                | ?                  | unique id generation                                                   | :heavy_check_mark: :white_square_button: :package: |
| [linenoise](https://github.com/antirez/linenoise)                  | ?                  | command line interface                                                 | :heavy_check_mark: :white_square_button: :package: |
| [lua](https://www.lua.org/)                                        | 5.1.5              | custom plugins                                                         | :heavy_check_mark: :package:                       |
| [cxxopts](https://github.com/jarro2783/cxxopts)                    | ?                  | command line option parser                                             | :heavy_check_mark: :package:                       |
| [i2pd](https://github.com/PurpleI2P/i2pd)                          | latest             | network proxy                                                          | :o: :package:                                      |
| [fmt](https://github.com/fmtlib/fmt)                               | ?                  | log formatting                                                         | :heavy_check_mark: :package:                       |
| [mkp224o](https://github.com/cathugger/mkp224o)                    | ?                  | onion address generation                                               | :o: :package:                                      |
| [protobuf](https://github.com/protocolbuffers/protobuf)            | ?                  | serialization for data transmission                                    | :heavy_check_mark:                                 |

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
git clone --recurse-submodules https://github.com/layters/testshop.git
```
```bash
cd testshop
```


**2. Install dependencies**

Debian/Ubuntu
```bash
# neroshop
sudo apt install libcurl4-openssl-dev libssl-dev qtdeclarative5-dev qml-module-qt-labs-platform qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qtquick-shapes qml-module-qtquick-dialogs
# monero-cpp (monero)
sudo apt update && sudo apt install pkg-config libssl-dev libzmq3-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libexpat1-dev libpgm-dev qttools5-dev-tools libhidapi-dev libusb-1.0-0-dev libprotobuf-dev protobuf-compiler libudev-dev libboost-chrono-dev libboost-date-time-dev libboost-filesystem-dev libboost-locale-dev libboost-program-options-dev libboost-regex-dev libboost-serialization-dev libboost-system-dev libboost-thread-dev python3 ccache
```
Arch (missing Qt/QML libraries)
```bash
# neroshop
sudo pacman -Sy --needed curl openssl qt5-declarative
# monero-cpp (monero)
sudo pacman -Syu --needed boost openssl zeromq libpgm unbound libsodium libunwind xz readline expat gtest python3 ccache qt5-tools hidapi libusb protobuf systemd
```
Fedora (missing Qt/QML libraries)
```bash
# neroshop
sudo dnf install libcurl-devel openssl-devel
# monero-cpp (monero)
sudo dnf install boost-static libstdc++-static pkgconf boost-devel openssl-devel zeromq-devel openpgm-devel unbound-devel libsodium-devel libunwind-devel xz-devel readline-devel expat-devel gtest-devel ccache qt5-linguist hidapi-devel libusbx-devel protobuf-devel protobuf-compiler systemd-devel
```


**3. Install expat and unbound:**
```bash
cd external/monero-cpp/external/monero-project
```

```bash
wget https://github.com/libexpat/libexpat/releases/download/R_2_4_8/expat-2.4.8.tar.bz2
tar -xf expat-2.4.8.tar.bz2
sudo rm expat-2.4.8.tar.bz2
cd expat-2.4.8
./configure --enable-static --disable-shared
make
sudo make install
cd ../
```

```bash
wget https://www.nlnetlabs.nl/downloads/unbound/unbound-1.22.0.tar.gz
tar xzf unbound-1.22.0.tar.gz
sudo apt update
sudo apt install -y build-essential
sudo apt install -y libssl-dev
sudo apt install -y libexpat1-dev
sudo apt-get install -y bison
sudo apt-get install -y flex
cd unbound-1.22.0
./configure --with-libexpat=/usr --with-ssl=/usr --enable-static-exe
make
sudo make install
cd ../
```

<!-- git submodule update --init --force --recursive --> <!-- <= call this before building monero -->
> [!Tip]
> Avoid using the `-j$(nproc)` option if you have <= 4 CPU cores and <= 4GB RAM to prevent system crashes. Use `-j1` instead.

**4. Build monero-project to create .a libraries**
```bash
make release-static -j$(nproc)
```
```bash
cd ../../../../
```


**5. Build neroshop**

To build with [**CMake**](https://cmake.org/):

```bash
# Build external libraries
cd external/
cmake .
make -j$(nproc)
cd ..
```

```bash
# Build neroshop
cd build
cmake .. #-DNEROSHOP_BUILD_CLI=1 #-DNEROSHOP_BUILD_TESTS=1
make -j$(nproc)
```

```bash
# Run neroshop
./neroshop
```
> Other supported build systems: [`Meson`](https://mesonbuild.com/)

### Setting up i2p
1. Download the Java I2P from the [official website](https://geti2p.net/en/download)
2. After installation, open the terminal and start I2P with the following command: 
```bash
/home/$USER/i2p/i2prouter start
```
3. In your browser, visit http://127.0.0.1:7657/configclients. Scroll down to enable the **SAM application bridge** and then apply the changes
   
   SAM should now be enabled. Please wait a few minutes before starting neroshop


## Contributing
See [Wiki](https://github.com/layters/testshop/wiki)


## License
This project is licensed under the [GNU General Public License v3.0](LICENSE)


## Donations
**Monero (XMR):**
```
83QbQvnnyo7515rEnW8XwF1hbP5qMab6sHXFzP6pg3EKGscgXCbVjbt1FX5SF7AV9p4Ur1tiommuQSzrQQRHkZicVYu6j8Y
```
<p align="center">
    <a href="monero:83QbQvnnyo7515rEnW8XwF1hbP5qMab6sHXFzP6pg3EKGscgXCbVjbt1FX5SF7AV9p4Ur1tiommuQSzrQQRHkZicVYu6j8Y" target="_blank"><img src="assets/images/donate_xmr.png" width="128" height="128"></img></a>
</p>

**Wownero (WOW):**
```
WW2pQTQWHpyJf2CHrCmZG7Tn3zBnYRZTH8g4U3pSZf5s6xsTXrZc9odDWmrWzjRc9MMQWrKXxjHsRdzH5JpJ7kzx1jZuSVSfi
```
<p align="center">
    <a href="wownero:WW2pQTQWHpyJf2CHrCmZG7Tn3zBnYRZTH8g4U3pSZf5s6xsTXrZc9odDWmrWzjRc9MMQWrKXxjHsRdzH5JpJ7kzx1jZuSVSfi" target="_blank"><img src="assets/images/donate_wow.png" width="128" height="128"></img></a>
</p>

[**OpenAlias**](https://openalias.org/):
~`donate.neroshop.org` or `donate@neroshop.org`~


## Resources
- Website: [neroshop.org](https://neroshop.org/) (out of service)
- DHT Specification: [specs](https://github.com/layters/specs)
- Git Mirrors: 
    - [Codeberg](https://codeberg.org/layter/neroshop)
    - [Radicle](https://radicle.xyz): `rad:z2Y72SYpHTkiRXrn4hkZaf1VYhc7J`
- Lemmy: https://monero.town/c/neroshop
- Matrix: [#neroshop:matrix.org](https://matrix.to/#/#neroshop:matrix.org)


## Thanks
* [yuriio147](https://github.com/yuriio147)
* [u/EchoingCat](https://www.reddit.com/user/EchoingCat/)

[//]: # (git checkout -b main)
[//]: # (git add .gitignore .gitmodules assets/ cmake/ CMakeLists.txt external/ LICENSE meson.build meson.options proto/ qml/ qml.qrc README.md src/ tests/)
[//]: # (git commit -m"..."    or    git commit -a --allow-empty-message -m "")
[//]: # (sudo git push -u origin backup --force)
[//]: # (adding an external lib to submodules index: git submodule add <url> external/<folder>)
[//]: # (removing an external lib from submodules index: git rm -rf --cached external/<folder>)
