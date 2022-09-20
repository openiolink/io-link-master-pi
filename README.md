# Introduction
TODO: Give a short introduction of your project. Let this section explain the objectives or the motivation behind this project.

## Autors

- Daniel Kessler, Balluff
- Dominik Nille, Balluff
- Pascal Frei
- Markus Gafner, BFH

## Install
To use the application on RaspberryPi4B, the operating system [Raspbian](http://raspbian.org) is used. Also, the library [WiringPi](http://wiringpi.com) and the build tool [CMake](https://cmake.org) is required. The Library WiringPi is offical deprecated for buster and bullseye debian-based os.

This can get installed using the command:
```bash
sudo apt-get install mosquitto wiringpi cmake
```
If wiringpi is not available install the .deb file provided in this repo using the command:
```bash
sudo dpkg -i wiringpi-2.61-1.deb
```

After a successful installation, the RaspberryPi is ready to use.

## Build and compile the project
The easiest way to compile the project is, to copy all sources to the RaspberryPi, and compile the project on the RaspberryPi itself or use Visual Studio Code with a SSH Connection.

- Copy all source files to a folder you like, for example `/home/pi/projects/`
- With the SSH-connection, move to the folder you copied the files: `cd /home/pi/projects/`
- Initialize CMake: `cmake .`
- Build the binary: `make`

If every step was successful, in the project folder should be an executable file, for example `openiolink`. This one can be executed using `./openiolink`.

## API
toDo: Hier bitte alle APIs rein mit 
