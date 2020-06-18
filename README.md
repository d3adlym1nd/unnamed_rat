[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)[![Platforms: linux-64 | win-64](https://img.shields.io/badge/platform-linux--64%20|%20win--64-blue.svg)]
# unnamed_rat
Multiplatform Command-line Remote Access Tool (RAT)

### Dependencies
- libssl
- libnotify `if desktop notifications are enabled`

### Included functions
- Connection over TLS
- File transfer
- Interactive reverse shell
- Information gathering
- Download files from HTTP/HTTPS servers

## Misc
To enable colored output or desktop notification uncomment the followig lines respectively on `headers.hpp` file 
```cpp
#define _COLOR 
#define _NOTIFY
```
To use `notify` feature you have to install `libnotify-dev` on your system and use the make file `makenotify` locate in the server source directory.
Compile with `make -f makenotify`.
