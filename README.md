# nRF5 Bluetooth Low Energy GAP/GATT driver

## Introduction
pc-ble-driver is a serialization library over serial port required by the following repos:

* [pc-ble-driver-js  library](https://github.com/NordicSemiconductor/pc-ble-driver-js)

The library is included as a submodule by the repositories above, but it can be built as standalone.

# Building from source

## Dependencies
The following dependencies are required for building the Driver:

* [cmake](http://www.cmake.org/cmake/resources/software.html) (>=2.8.X)
* [Boost](http://www.boost.org/users/download) (>=1.56.0)

## Get the source code
The source code is available from GitHub at the following URL:
<https://github.com/NordicSemiconductor/pc-ble-driver.git>


## Build environment

### Windows 

- Install cmake according to the [installation instructions for Windows](http://www.cmake.org/install/).
- Download and extract boost. We assume Boost is extracted into `c:\boost\boost_1_xx_y`.

#### Using MinGW
- Download the [MinGW] (http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.8.2/threads-posix/dwarf/) Compiler Suite.
- Install the MinGW Compiler Suite according to the [instructions](http://www.mingw.org/wiki/InstallationHOWTOforMinGW).

#### Using Microsoft Visual Studio
- Install the Microsoft Visual Studio version of your choice.

#### Building boost with MinGW

Open a MinGW terminal and issue the following commands in the directory:

    $ cd c/boost/boost_1_xx_y
    $ ./bootstrap.sh
    $ ./b2 toolset=gcc address-model=32 link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

#### Building boost with Visual Studio

Open a Microsoft Visual Studio Command Prompt and issue the following commands in the directory:

    > cd c:\boost\boost_1_xx_y
    > bootstrap.bat
    > b2 toolset=msvc-VV.V address-model=[32,64] link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Refer to the [compiler list](http://www.boost.org/build/doc/html/bbv2/reference/tools.html#bbv2.reference.tools.compilers) of the boost documentation 
to find the version of the MSVC that you need to provide using the `toolset=` option.


### Ubuntu Linux
Run the following command to install compiler, dependencies and tools:

    curl -L https://raw.githubusercontent.com/NordicSemiconductor/pc-ble-driver/master/scripts/setup-ubuntu-linux.sh | sh
    
You will be asked for root password during this process.

### OS X
Run the following command to install compiler, dependencies and tools:

    curl -L https://raw.githubusercontent.com/NordicSemiconductor/pc-ble-driver/master/scripts/setup-osx.sh | sh

You will be asked for root password during this process.

