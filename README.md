# fb_dbcommands

A legacy commandline tool for loading and saving databases of the [ACPLT/OV runtime environment](https://github.com/acplt/rte) via the ACPLT/KS protocol


## Build Requirements
* cmake
* C++-Compiler
* bison
* flex
* RPC library (if not included in libc)

### Build options
```
BUILD_DYNAMIC       switch between static and dynamic linking builds (default: static)
RPC_LIB_NAME        name of separate RPC library (e.g. tirpc)
RPC_INCLUDE_PATH    path to header files of a separate RPC library
```

### Building

Get a recursive clone of this repository:
```shell
git clone --recursive https://github.com/acplt/dbcommands.git
```

Create a build folder in repository root and generate build files:
```shell
cd dbcommands
mdkir build && cd build
cmake ..
```

Optionally use ccmake or arguments in cmake call to set build options.  
Finally, build executable fb_dbcommands:
```shell
make -j4 fb_dbcommands
```

## Usage

```
Usage: fb_dbcommands [arguments]

The following optional arguments are available:
-s            HOST[/SERVER]  Set host and server name (default 'localhost/fb_database')
-f            NAME           Set filename NAME.fbd (default HOST_SERVER.fbd)
-t            TICKET         Set AV-ticket
-save                        Save database
-clean                       Clean database
-load                        Load database
-reload                      Save, clean and load database
-upload       LIB_PATH       upload library 'LIB_PATH'
-uploadPwd    PASSWORD       password for replace library
-all                         Save, clean or load all fb-server on host HOST (option "-s HOST")
-nolog                       Do not protocol file
-h OR --help                 Display this help message and exit
```

### Example usage

load the database save <fbd-file> into the `MANAGER` server on localhost:  
`fb_dbcommands -load -s localhost/MANAGER -f <fbd-file>`