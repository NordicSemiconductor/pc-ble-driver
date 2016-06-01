# nRF5 Bluetooth Low Energy GAP/GATT driver

## Introduction
pc-ble-driver is a serialization library over serial port required by the following repos:

* [pc-ble-driver-js  library](https://github.com/NordicSemiconductor/pc-ble-driver-js)
* [pc-ble-driver-py  library](https://github.com/NordicSemiconductor/pc-ble-driver-py)

The library is included as a submodule by the repositories above, and it should be built as part of them.

# Building boost

The boost static libraries required by this drivers must be build before you can use 

## Obtain the boost source code

Use the following link to download the boost source code:

* [Boost](http://www.boost.org/users/download) (>=1.54.0)

### Windows 

- Download and extract boost. We assume Boost is extracted into `c:\boost\boost_1_xx_y`.
- Set the environment variable `BOOST_ROOT` to the path of your boost library location above.

#### Building boost with MinGW

- Download the [MinGW] (http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.8.2/threads-posix/dwarf/) Compiler Suite.
- Install the MinGW Compiler Suite according to the [instructions](http://www.mingw.org/wiki/InstallationHOWTOforMinGW).

Open a MinGW terminal and issue the following commands in the directory:

    $ cd c/boost/boost_1_xx_y
    $ ./bootstrap.sh
    $ ./b2 toolset=gcc address-model=32 link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

#### Building boost with Visual Studio

Install Microsoft Visual Studio. The following versions supported are:

* Visual Studio 2013 (MSVC 12.0)
* Visual Studio 2015 (MSVC 14.0)

Open a Microsoft Visual Studio Command Prompt and issue the following commands in the directory:

    > cd c:\boost\boost_1_xx_y
    > bootstrap.bat
    > b2 toolset=msvc-VV.V address-model=[32,64] link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Refer to the [compiler list](http://www.boost.org/build/doc/html/bbv2/reference/tools.html#bbv2.reference.tools.compilers) of the boost documentation 
to find the version of the MSVC that you need to provide using the `toolset=` option.

### Ubuntu Linux
TBD

### OS X
TBD

