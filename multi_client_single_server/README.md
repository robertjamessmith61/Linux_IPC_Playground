# Multi Client Single Server

This is a basic example of a single server which pushes messages to multiple clients.

## Description

### fifo_server
The fifo_server app prompts the user for input and then sends this text over named pipes to each subscribed client. It creates a named pipe which clients can send the name of their own named pipes to and be added to the subscriber list.

### fifo_client
The fifo_client app creates its own named pipe and then sends the name to the server over the servers common named pipe. It then reads from its own named pipe and prints to the screen.

## Build

Build both fifo_server and fifo_client apps using cmake:

```bash
if [ ! -d build ]; then mkdir build; fi && cd build && cmake .. && make && cd ..
```

Alternatively you can call either the debug.sh or release.sh scripts to build both apps.

```bash
./debug.sh
# OR
./release.sh
```

## Running the programs

Start the fifo_server process first:

```bash
./build/fifo_server
```

Then start multiple fifo_client processes in separate terminals:

```bash
./build/fifo_client
```

Type some text in the fifo_server terminal and press enter:

```
Type message and press enter (type exit to close app):
example text
Message:
example text
```

You should see the text you typed printed out by all of the reader processes.

```
recieved:
example text
```
