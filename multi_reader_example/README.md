# Multi Reader Example

This is a small test project to test behaviour when multiple processes read from the same named pipe.

## Description

### fifo_sender
The fifo_sender app prompts the user for input and then sends this text over a named pipe.

### fifo_reader
The fifo_reader app simply reads from the named pipe and prints to the screen.

## Build

Build both fifo_sender and fifo_reader apps using gcc:

```bash
gcc -o fifo_sender fifo_sender.c -pthread && gcc -o fifo_reader fifo_reader.c -pthread
```

## Running the programs

Start multiple fifo_reader processes in separate terminals first (You need at least two to test behaviour):

```bash
./fifo_reader
```

Then start fifo_sender in a separate terminal and when prompted type some text and press enter:

```bash
./fifo_sender

Type message and press enter (type exit to close app):
example text
Message:
example text
```

You should see the text you typed printed out by only one of the reader processes.

```bash
recieved:
example text
```
If you input some new text and press enter, you should see that text printed out by a different reader process. The reader processes should take it in turns to read from the named pipe.