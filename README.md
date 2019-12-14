# publish-subscribe

This project implements basic Publish-Subscribe communication protocol on TCP/IP layer. Client and Server console apllications were developed in C.
Server opens TCP socket on `localhost:port` and waits for connections.
Clients are connecting to the server, subscribing to topics and publishing topic releated information (see Client Commands).
Server takes care of clients and their subscriptions, and informs them as soon as a topic of interest is published.

## Getting Started

Clone repository to a Linux machine.

### Prerequisites

Linux machine with GCC 4.9 and up.
To check GCC version run
```
$ gcc --version | grep ^gcc | sed 's/^.* //g'
```

### Building binaries

To build Server & Client binaries, in root directory, run

```
$ make
```
Objects and binaries are stored in

```
bin/
build/
```

To rebuild binaries run

```
$ make remake
```

To clean objects and binaries run

```
$ make clean
```

## Running the tests

To test publish-subscribe communication run server and multiple clinets.

How to start server and clients:

First tab:
```sh
$ ./bin/server 8080
```
Second and other tabs:
```sh
$ ./bin/client
```
Once server is started, clients can connect to it.
### Client Commands

|Command                          |Example                      |Description                  |
|---------------------------------|-----------------------------|-----------------------------|
|`CONNECT:<port>:<name>`          |`CONNECT:8080:Client1`       |Connects Client1 with server |
|`SUBSCRIBE:<topic>`              |`SUBSCRIBE:Topic1`           |Subscribe to Topic1          |
|`PUBLISH:<topic>:<data to share>`|`PUBLISH:Topic1:Hello there!`|Client publishs data related to Topic1  |
|`UNSUBSCRIBE:<topic>`            |`UNSUBSCRIBE:Topic1`         |Unsubscribe from Topic1      |
|`DISCONNECT`                     |`DISCONNECT`                 |Disconnects from server      |

### Server Commands

|Command    |Example |Description  |
|-----------|--------|-------------|
|`exit`     |`exit`  | Kill server |

## Author

* **Ivan Lasic** - [laske992](https://github.com/laske992)

