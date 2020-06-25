[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) ![Platforms: linux-64 | win-64](https://img.shields.io/badge/platform-linux--64%20|%20win--64-success.svg)
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

### Misc
To enable colored output or desktop notification on server uncomment the followig lines respectively on `headers.hpp` file 
```cpp
#define _COLOR 
#define _NOTIFY
```
To use `notify` feature you have to install `libnotify` on your system and use the make file `makenotify` locate in the server source directory.
Compile with `make -f makenotify`.

### How-To (Client)
Edit `headers.hpp` and comment or uncomment `#define _DEBUG` to enable/disable verbose output to console. Modify `main.cpp` and change the line
```cpp
Cli->Connect("YOUR HOST", "PORT")
```
with your host and port information. If `#define _DEBUG` is uncommented then host and port information must be passed as arguments from command-line. Build in client root directory with `make`.

### How-To (Server)
Generate certificate and private key on server with `openssl req -x509 -newkey rsa:4096 -out cacer.pem -outform PEM -days 1825 -nodes` and place both files `cacer.pem` and `prikvey.pem` on server directory.
Edit `headers.hpp` and modify the line `#define Max_Clients 10` if you want to handle more than 10 clients. Build in server root directory with `make`.

### Server Commands
Type `help`, `?` or `aiuda` at any time (not in reverse shell) to show available commands

#### Main Shell
- cli - Execute actions on clients
- help|?|aiuda - Show available commands
- exit - Finish server

#### Client Shell
- upload - Upload a local file to remote client
- download - Download a remote file from client
- shell - Spawn an interactive reverse shell on remote client
- httpd - Force client to download a file from specified url
- info - Retrieve information from client
- help|?|aiuda - Show available commands
- exit - Close interactive session
- More comming...

## Tested on
- Debian
- Arch
- Windows 7
- Windows 10

## Portable Windows Client
- Working on

## Contribute
Any contribution its welcome!!!

# DISCLAIMER
This software is for testing purposes only, designed to run in a controlled environment, it should not be run in a real-world scenario. The developer is not responsible for any damage caused or legal repercussions. Use it at your own risk.
