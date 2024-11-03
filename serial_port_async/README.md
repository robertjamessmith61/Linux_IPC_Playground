# Serial Port Async Example

This is a small example project for a basic async serial port program.

## Description

The serial_port_async program opens the serial port specified as the first argument and allows the user to send bytes input as hex characters from stdin. It prints whatever it recieves in response to stdout.

## Build

Build both fifo_sender and fifo_reader apps using gcc:

```bash
if [ ! -d bin ]; then mkdir bin; fi && gcc -o bin/fifo_sender fifo_sender.c -pthread && gcc -o bin/fifo_reader fifo_reader.c -pthread
```

## Build

Build the app using cmake:

```bash
if [ ! -d build ]; then mkdir build; fi && cd build && cmake .. && make && cd ..
```

Alternatively you can call either the debug.sh or release.sh scripts to build the app.

```bash
./debug.sh
# OR
./release.sh
```

## Running the program

Start the serial_port_async process with a serial device as first argument:

```bash
./bin/serial_port_async [ serial device ]

# Example
./bin/serial_port_async /dev/ttyS1
# OR
./bin/serial_port_async /dev/ttyUSB1
```

<div align="center">

---

**--- NOTE ---**

**THIS PROGRAM RELIES ON HAVING A DEVICE CONNECTED TO THE SERIAL PORT WHICH COMMUNICATES USING A BINARY PROTOCOL OF RAW BYTES. THIS WILL AFFECT WHAT IS READ BACK FROM THE DEVICE.**

**IF A DEVICE IS NOT CONNECTED YOU CAN INSTEAD LOOPBACK THE RX AND TX LINES OF THE SERIAL PORT SO THAT YOU RECIEVE AN ECHO OF WHAT IS SENT.**

---

</div>

Type some hex bytes in the terminal and press enter:

```bash
# Assuming serial lines are looped back.
Type message and press enter (type exit to close app):
48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21
Message:
48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21
```
