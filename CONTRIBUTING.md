There are many ways that you can contribute.

- Creating [pull requests](https://github.com/larteyoh/testshop/pulls) that solve issues found on the [issues page](https://github.com/larteyoh/testshop/issues)
- Making a [donation](https://github.com/larteyoh/testshop#donations) to fund the development of the project
- Reporting any bugs or vulnerabilities found in the code as a GitHub [issue](https://github.com/larteyoh/testshop/issues)
- Sharing this project so more people can learn about its existence

**Please refer to [the wiki](https://github.com/larteyoh/testshop/wiki/FAQ#how-can-i-contribute-to-neroshop-if-i-dont-know-c-or-c) for more information on how to contribute.**


# Testing
Also, If you would like to help test the P2P network, you can skip to this [section](#testing-dht)

### Testing RPC

To run the RPC server, use the following command:
```
./neromon --enable-rpc
```

Then you can open a browser and test the RPC network by typing `http://127.0.0.1:57741` in the address bar.

A JSON-RPC error response should appear.



### Testing IPC

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

### Testing DHT

To start the network, you must run your node as a bootstrap node:
```
./neromon --bootstrap
```
Then in another terminal tab or window, open up neromon again, but this time without the bootstrap option:
```
./neromon
```

The bootstrap node should then receive a `ping` and will respond with a `pong`. On receiving a ping, the bootstrap node will add the sender node to its own routing table. The sender node will also add the bootstrap node to its own routing table as soon as it receives the pong from the bootstrap node.

Afterwards, the sender node will send a `find_node` message to the bootstrap node and the bootstrap node will respond with a `nodes` message containing the closest nodes in the bootstrap node's routing table, which the sender node will also store in its own routing table.

The is what is called the `join` process and it will be how all nodes join the network.


### Running a public node
To run a public node, you must include the `public` flag when starting the daemon:
```
./neromon --public
```

Be sure that port forwarding is enabled on your machine in order for other nodes to find your node.

Fun fact: You can also run a public RPC server by combining the `--rpc` and `--public` flags.
