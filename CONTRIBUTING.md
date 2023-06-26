There are many ways that you can contribute.

- Creating [pull requests](https://github.com/larteyoh/testshop/pulls) that solve issues found on the [issues page](https://github.com/larteyoh/testshop/issues)
- Making a [donation](https://github.com/larteyoh/testshop#donations) to fund the development of the project
- Help us [test](#testing) the software and report any bugs or vulnerabilities found in the code as a GitHub [issue](https://github.com/larteyoh/testshop/issues)
- Sharing this project so more people can learn about its existence
- Support the neroshop network by running your own neroshop node or volunteering to become a bootstrap node

**Please refer to [the wiki](https://github.com/larteyoh/testshop/wiki/FAQ#how-can-i-contribute-to-neroshop-if-i-dont-know-c-or-c) for more information on how to contribute.**



# Testing
Also, If you would like to help test the P2P network, you can skip to this [section](#testing-dht)

## Testing GUI
To run the GUI application, use the following command:
```
./neroshop
```
By default, the GUI runs the daemon in a separate detached process so there's no need to worry about that unless you're using `neroshop-console`, but if you really need to observe what is going on behind the scenes, you can still launch the daemon from within the terminal before launching the GUI application.

The GUI should automatically detect whether the daemon is running in the background or not.

**Both `neroshop` and `neroshop-console` cannot be opened simultaneously as the IPC server only accepts a single client connection at a time.**



## Testing RPC

To run the RPC server, use the following command:
```
./neromon --enable-rpc
```

Then you can open a browser and test the RPC network by typing `http://127.0.0.1:50882` in the address bar.

A JSON-RPC error response should appear.



## Testing IPC

To run the IPC server normally, use the following command:
```
./neromon
```
This should start up the IPC server. Then you can proceed to launch neroshop-console, which will act as the client and then send a message to the IPC server:
```
./neroshop-console
> send
```

From there, you should receive a pong message back from the server.



## Testing DHT

To start the network, you must run your node as a bootstrap node (assuming your node's IP is hard-coded into the software):
```
./neromon --bootstrap
```
Then in another terminal tab or window, open up neromon again, but this time without the bootstrap option:
```
./neromon
```

The bootstrap node should then receive a `ping` and will respond with a `pong`. On receiving a ping, the bootstrap node will add the sender node to its own routing table. The sender node will also add the bootstrap node to its own routing table as soon as it receives the pong from the bootstrap node.

Afterwards, the sender node will send a `find_node` message to the bootstrap node and the bootstrap node will respond with a `nodes` message containing the closest nodes in the bootstrap node's routing table, which the sender node will also contact and then store in its own routing table.

The is what is called the `join` process and it will be how all nodes join the network.


### Running a public node
To run a public node, you must include the `--public` flag when starting the daemon:
```
./neromon --public
```

**Be sure that port forwarding is enabled on your machine in order for other nodes to find your node on port `50881`.**

Another thing: You can also run a public RPC server by combining the `--rpc` and `--public` flags.



# Releasing
## AppImage
### Prerequisites
In order to generate AppImage files for use on all Linux distributions, the following is required:
- [`linuxdeploy`](https://github.com/linuxdeploy/linuxdeploy/releases)
- [`AppImageKit`](https://github.com/AppImage/AppImageKit/releases)


### Generating AppImages from native binaries
Assuming you've already built all of the external libraries from `external/`, you can proceed to build neroshop

If you have already built the neroshop binaries, you may need to `clean` the Makefile and remove the CMake files before we can proceed.

First, lets make sure we are inside the `build` directory within the project's root directory:
```
cd build
```

Now we can build neroshop in preparation for the AppImage generation:
```sh
# configure build system
# the flags below are the bare minimum that is needed, the app might define additional variables that might have to be set
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DNEROSHOP_BUILD_CLI=1
 
# build the application on all CPU cores
make -j$(nproc)
 
# now "install" resources into future AppDir
make install DESTDIR=neromon.AppDir
make install DESTDIR=neroshop.AppDir
make install DESTDIR=neroshop-console.AppDir

# remove all excess/unnecessary files
sudo rm 'neromon.AppDir/usr/bin/neroshop' 'neromon.AppDir/usr/bin/neroshop-console' 'neromon.AppDir/usr/share/applications/neroshop.desktop' 'neromon.AppDir/usr/share/applications/neroshop-console.desktop'
sudo rm 'neroshop.AppDir/usr/bin/neromon' 'neroshop.AppDir/usr/bin/neroshop-console' 'neroshop.AppDir/usr/share/applications/neromon.desktop' 'neroshop.AppDir/usr/share/applications/neroshop-console.desktop'
sudo rm 'neroshop-console.AppDir/usr/bin/neromon' 'neroshop-console.AppDir/usr/bin/neroshop' 'neroshop-console.AppDir/usr/share/applications/neromon.desktop' 'neroshop-console.AppDir/usr/share/applications/neroshop.desktop'
```

Run the following command to copy the required shared libraries to AppDir:
```sh
./linuxdeploy-x86_64.AppImage --appdir ./neromon.AppDir --desktop-file=../assets/neromon.desktop
./linuxdeploy-x86_64.AppImage --appdir ./neroshop.AppDir --desktop-file=../assets/neroshop.desktop
./linuxdeploy-x86_64.AppImage --appdir ./neroshop-console.AppDir --desktop-file=../assets/neroshop-console.desktop
```

Finally, we generate an AppImage from the AppDir:
```sh
./appimagetool-x86_64.AppImage neromon.AppDir neromon.AppImage
./appimagetool-x86_64.AppImage neroshop.AppDir neroshop.AppImage
./appimagetool-x86_64.AppImage neroshop-console.AppDir neroshop-console.AppImage
```

### References
https://docs.appimage.org/packaging-guide/from-source/native-binaries.html


