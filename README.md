# Linux IPC Playground

This is a simple repository for storing examples and test applications I have used when experimenting with Linux inter-process communication.

## Description

Feel free to copy and reuse any of this code. I have based everything off the very good opensource.com guide on Linux IPC (Links below under Acknowledgments). There may be some small details which are out of date in the guide which I have changed to try and fit best practice, but I am not an expert so feel free to point out any issues.

## Getting Started

### Dependencies

* Pretty much any distribution of Linux.

### Installing

* Download the folder for the example you want.
* Build the example using gcc:
```bash
gcc -o fifo_send fifo_send.c -pthread
```

### Executing program

* Run the example from the terminal with any arguments required.
```bash
./fifo_send 100
```

## Help

Try running an example without arguments to print usage.
You can also look at the source files or read the README in the specific example sub-folder for more.

## Authors

* Robert Smith | 
[LinkedIn](https://www.linkedin.com/in/engineerrobertsmith?lipi=urn%3Ali%3Apage%3Ad_flagship3_profile_view_base_contact_details%3BSqaEIr%2FISwqf0CrgCsD2Kw%3D%3D)

## License

This project is licensed under the [Unlicense](https://unlicense.org/) License - see the LICENSE.md file for details

## Acknowledgments

* [inter-process_communication_in_linux.pdf](https://opensource.com/sites/default/files/gated-content/inter-process_communication_in_linux.pdf)
* [Martin Kalin](https://condor.depaul.edu/mkalin/)
* [Matrin Kalin's Source code for the series on IPC in Linux](https://condor.depaul.edu/mkalin/ipcCode.zip)