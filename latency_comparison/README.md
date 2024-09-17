# Latency Comparison

This is a small test project to measure latency over a pair of named pipes.

## Description

### fifo_send
The fifo_send app sends random bytes over one named pipe and reads them back over the other. It does this for a number of loops specified by the user before exiting. Latency is measured by comparing the time before and after completing all the loops. As this is roundtrip latency, single ended latency can be estimated by halving the number.

### fifo_echo
The fifo_echo app should be started first. It simply reads out from one pipe and sends it back over the other.

## Build

Build both fifo_send and fifo_echo apps using gcc:

```bash
gcc -o fifo_send fifo_send.c -pthread && gcc -o fifo_echo fifo_echo.c -pthread
```

## Running the programs

Start fifo_echo in one terminal first:

```bash
./fifo_echo
```

Then start fifo_send in a separate terminal and specify the number of loops to run:

```bash
./fifo_send 100
```
