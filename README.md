[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) ![Platforms: linux-64 | win-64 | android 9](https://img.shields.io/badge/platform-linux--64%20|%20win--64%20|%20android-success.svg)

![Documentacion en Español](https://github.com/d3adlym1nd/unnamed_rat/blob/master/README.es.md)
# unnamed_rat
Multiplatform Command-line Remote Access Tool (RAT)

### Dependencies
- libssl
- libnotify (Desktop notifications) - Optional

### Client Included functions
- Connection over TLS
- File transfer
- Interactive reverse shell
- Information gathering
- Download files from HTTP(S) servers

### Language
Edit `headers.hpp` and uncomment the language you want to use in the program. Languages so far:
```cpp
//Spanish
#define ES

//English
#define EN
```

### Misc
To enable colored output or desktop notification(Linux) on server uncomment the followig lines respectively on `headers.hpp` file 
```cpp
#define _COLOR

//Linux only 
#define _NOTIFY
```
To use `notify` feature you have to install `libnotify` on your system and use the make file `makenotify` locate in the server source directory.
Compile with `make -f makenotify`.

### Building (Client)
Edit `headers.hpp` and comment/uncomment:
```cpp
#define _DEBUG
```
 to enable/disable verbose output to console. Modify `main.cpp` and change the line
```cpp
Cli->Connect("YOUR HOST", "PORT")
```
with your host and port information. If `#define _DEBUG` is uncommented then host and port information must be passed as arguments from command-line. Ex `./Client 127.0.0.1 31337`. Build in client root directory with `make`.

### Building (Server)
Generate certificate and private key on server with `openssl req -x509 -newkey rsa:4096 -out cacer.pem -outform PEM -days 1825 -nodes`, place both files `cacer.pem` and `prikvey.pem` on server directory.
Edit `headers.hpp` and modify the line 
```cpp
#define Max_Clients 10
```
 if you want to handle more than 10 clients. Build in server root directory with `make`.

### Building Server/Client on Windows
First install Openssl using one of the pre-compiled binaries from here: https://wiki.openssl.org/index.php/Binaries. Download msys2 from here: https://www.msys2.org/. Open the msys2 shell and update packages list with `pacman -Syu`, after that, install requiered packages with: `pacman -S mingw-w64-x86_64-toolchain`, then compile with `mingw32-make`.


### Server (Termux)
To use under termux you have to install dependencies with:
```sh
pkg install openssl openssl-tool make git nano
```
After that edit `headers.hpp` with `nano` and uncomment 
```cpp
#define _TERMUX
```
 generate certificate and private key as explained above, finally compile with `make`.

### Server Commands
Type `help`, `?` or `aiuda` at any time (not in reverse shell) to show available commands

#### Main Shell
Command | About
------- | -----
cli | Execute actions on clients
help, ?, aiuda | Show available commands
exit | Finish server

#### Client Shell (Interactive Mode)
Command | About
------- | -----
upload | Upload a local file to remote client
download | Download a remote file from client
shell | Spawn an interactive reverse shell on remote client
httpd | Force client to download a file from specified url
info | Retrieve information from client
help, ?, aiuda | Show available commands
exit | Close interactive session (Not connection)
- More comming...

## Tested on
- Debian
- Arch
- Windows 7
- Windows 10
- Android 9 (Termux)

## Contribute
Any contribution its welcome!!!

## Screenshots
![](https://i.imgur.com/r6FewoQ.jpg)
![](https://i.imgur.com/fUgwlZx.jpg)
![](https://i.imgur.com/AZqPXmg.jpg)

![](https://i.imgur.com/p04wBN1.jpg)

![](https://i.imgur.com/NF7cQUC.jpg)

![](https://i.imgur.com/7Q4yjxh.jpg)

![](https://i.imgur.com/TNRV7kh.jpg)

# DISCLAIMER
This software is for testing purposes only, designed to run in a controlled environment, it should not be run in a real-world scenario. The developer is not responsible for any damage caused or legal repercussions. Use it at your own risk.
