# Multi Reader/Writer Example

This is a small test project to test behaviour when multiple processes read from or write to the same named pipe.

## Description

### fifo_sender
The fifo_sender app prompts the user for input and then sends this text over a named pipe.

### fifo_reader
The fifo_reader app simply reads from the named pipe and prints to the screen.

### Closing vs unlinking pipes
When a process ends it should call close() on the pipe file descriptor so that the OS knows it is no longer using it. When all write processes have called close() then an EOF will be sent.

You should not call unlink() until you are sure all processes have finished using the pipe. This removes the link to the filename which is required for any new process to open the pipe.

For example if I have one reader and multiple sender processes. I would only call unlink() from the reader process when it is finished.

## Build

Build both fifo_sender and fifo_reader apps using gcc:

```bash
if [ ! -d bin ]; then mkdir bin; fi && gcc -o bin/fifo_sender fifo_sender.c -pthread && gcc -o bin/fifo_reader fifo_reader.c -pthread
```

## Running the programs

### Multiple readers

Start multiple fifo_reader processes in separate terminals first (You need at least two to test behaviour):

```bash
./bin/fifo_reader
```

Then start fifo_sender in a separate terminal and when prompted type some text and press enter:

```bash
./bin/fifo_sender
```
```
Type message and press enter (type exit to close app):
example text
Message:
example text
```

You should see the text you typed printed out by only one of the reader processes.

```
recieved:
example text
```
If you input some new text and press enter, you should see that text printed out by a different reader process. The reader processes should take it in turns to read from the named pipe.

### Multiple writers

Follow the instructions above except:
* Start multiple fifo_sender processes in separate terminals and only one fifo_reader process.
* Each time you type something and hit enter on one of the sender processes you should see it printed out by the reader.