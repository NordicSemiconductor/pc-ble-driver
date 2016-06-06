# nRF5 Bluetooth Low Energy GAP/GATT driver

## Introduction
pc-ble-driver is a serialization library over serial port required by the following repos:

* [pc-ble-driver-js  library](https://github.com/NordicSemiconductor/pc-ble-driver-js)
* [pc-ble-driver-py  library](https://github.com/NordicSemiconductor/pc-ble-driver-py)

The library is included as a submodule by the repositories above, and it should be built as part of them.

# Installing and building Boost

The Boost static libraries required by this drivers must be built before you can build any of the
repositories above that depend on pc-ble-driver.

## Obtain the Boost source code

Note: This step is not required for OS X.

Use the following link to download the Boost source code:

* [Boost](http://www.boost.org/users/download) (>=1.54.0)

- Download and extract Boost to a folder of your choice.
- Set the environment variable `BOOST_ROOT` to the path where you have extracted Boost.

For example on Windows assuming you've unpacked Boost in `c:\boost\boost_1_xx_y`:

    BOOST_ROOT = "c:\boost\boost_1_xx_y"

And on Linux or OS X assuming you've unpacked Boost in `~/boost/boost_1_xx_y`:

    BOOST_ROOT = "~/boost/boost_1_xx_y"

### Windows 

#### Building Boost with MinGW

- Download the [MinGW] (http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.8.2/threads-posix/dwarf/) Compiler Suite.
- Install the MinGW Compiler Suite according to the [instructions](http://www.mingw.org/wiki/InstallationHOWTOforMinGW).

Open a MinGW terminal and issue the following commands:

    $ cd $BOOST_ROOT
    $ ./bootstrap.sh
    $ ./b2 toolset=gcc address-model=32 link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

#### Building Boost with Visual Studio

Install Microsoft Visual Studio. The following versions supported are:

* Visual Studio 2013 (MSVC 12.0)
* Visual Studio 2015 (MSVC 14.0)

Open a Microsoft Visual Studio Command Prompt and issue the following commands:

    > cd %BOOST_ROOT%
    > bootstrap.bat
    > b2 toolset=msvc-VV.V address-model=[32,64] link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Refer to the [compiler list](http://www.boost.org/build/doc/html/bbv2/reference/tools.html#bbv2.reference.tools.compilers) of the Boost documentation 
to find the version of the MSVC that you need to provide using the `toolset=` option.

### Ubuntu Linux

Install the required packages to build Boost:

    sudo apt-get install git make gcc g++

Open a terminal window and issue the following commands:

    $ cd $BOOST_ROOT
    $ ./bootstrap.sh
    $ ./b2 toolset=gcc cxxflags=-fPIC cflags=-fPIC address-model=[32,64] link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: Select 32 or 64-bit with the `address-model=` option.

### OS X 10.11 and later

Install Xcode from the App Store.

The simplest way to install Boost is to use Homebrew. If you don't have Homebrew installed simply run on a terminal:

    $ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you already have Homebrew installed, make sure it's up to date with:
  
    $ brew update
    $ brew upgrade

Once Homebrew is installed you can use the `brew` command on a terminal to install

    $ brew install boost

