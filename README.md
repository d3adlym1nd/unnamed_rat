[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
# unnamed_rat
Command-line Remote Access Tool (RAT)

### Included functions
- Connection over TLS
- File transfer
- Interactive reverse shell
- Information gathering
- Download files from HTTP/HTTPS servers

## Misc
To enable colored output uncomment 
```cpp
#define _COLOR 
```
on the `Headers.hpp` file.

To enable desktop notifications uncomment
```cpp
#define _NOTIFY
```
on the `Headers.hpp` file. To use this feature you have to install `libnotify-dev` on your system and use the make file `makenotify` locate in the server source directory.
Compile with `make -f makenotify`.
