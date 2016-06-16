# nRF5 Bluetooth Low Energy GAP/GATT driver

## Introduction
pc-ble-driver is a static and shared library that provides serial port communication (using SoftDevice API serialization) to an nRF5x IC running the connectivity firmware included.
It is a C/C++ library that can be interfaced directly but it is more often used by higher level bindings:

* [pc-ble-driver-js  JavaScript bindings](https://github.com/NordicSemiconductor/pc-ble-driver-js)
* [pc-ble-driver-py  Python bindings](https://github.com/NordicSemiconductor/pc-ble-driver-py)

The library is included as a submodule by the repositories above, and it should be built as part of them.

## SoftDevice and IC Support

The library is compatible with the following SoftDevice API versions and nRF5x ICs:

* s130_nrf51_2.x.x (nRF51 series ICs)
* s132_nrf52_2.x.x (nRF52 series ICs)

The .hex files included in the `hex/` folder include both the SoftDevice and the connectivity firmware required to communicate with it.

## Operating system support

* Windows (XP, 7, 8, 8.1, 10) 32 and 64-bit
* GNU/Linux (Ubuntu tested) 32 and 64-bit
* macOS (OS X) 32 and 64-bit

## Hardware setup

### Installing drivers and tools

This communication library works over any kind of serial port (UART), but it is most often used over a Segger J-Link USB CDC UART.
To set up the required J-Link drivers simply download and install the version matching you operating system:

* [Segger J-Link Downloads](https://www.segger.com/jlink-software.html)  

Additionally to flash the connectivity firmware you will need `nrfjprog` which is bundled with the nRF5x Command-Line Tools, which can be downloaded from:

* [nRF5x Command-Line Tools for Windows](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444)
* [nRF5x Command-Line Tools for Linux 32-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux32/52615)
* [nRF5x Command-Line Tools for Linux 64-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386)
* [nRF5x Command-Line Tools for OS X](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402)

### Flashing the connectivity firmware

To use this library you will need to flash the connectivity firmware on a nRF5x IC

Once you have installed the nRF5x Command-Line Tools, you can erase and program the IC:

    $ nrfjprog -f NRF5<x> -e
    $ nrfjprog -f NRF5<x> --program hex/connectivity_115k2_with_s13x_2.<x>.<x>.hex

### J-Link USB CDC serial ports

After you have installed the required drivers and connected a J-Link enabled board (such as the Nordic Development Kits) the port should appear automatically

#### Windows

The serial port will appear as `COMxx`. Simply check the "Ports (COM & LPT)" section in the Device Manager.

#### Ubuntu Linux

The serial port will appear as `/dev/ttyACMx`. By default the port is not accessible to all users, type this command to add your user to the `dialout` group to give it acces to the serial port:

    sudo usermod -G dialout <username>

#### OS X

The serial port will appear as `/dev/tty.usbmodemXXXX`.

## Building Boost

The Boost static libraries required by this drivers must be built before you can build any of the
repositories above that depend on pc-ble-driver.

### Obtain the Boost source code

Note: This step is not required for OS X.

Use the following link to download the Boost source code:

* [Boost](http://www.boost.org/users/download) (>=1.54.0)

- Download and extract Boost to a folder of your choice.
- Set the environment variable `BOOST_ROOT` to the path where you have extracted Boost.

For example on Windows assuming you've unpacked Boost in `c:\boost\boost_1_xx_y`:

    BOOST_ROOT = "c:\boost\boost_1_xx_y"

And on Linux or OS X assuming you've unpacked Boost in `~/boost/boost_1_xx_y`:

    BOOST_ROOT = "~/boost/boost_1_xx_y"

#### Windows 

Install Microsoft Visual Studio. The following versions supported are:

* Visual Studio 2013 (MSVC 12.0)
* Visual Studio 2015 (MSVC 14.0)

Open a Microsoft Visual Studio Command Prompt and issue the following commands:

    > cd %BOOST_ROOT%
    > bootstrap.bat
    > b2 toolset=msvc-<VV.V> address-model=<32,64> link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: If you intend to build a 64-bit version of Boost, and depending on the version of Visual Studio you use, you might need to open a 64-bit command prompt such as
"Visual Studio 2015 x86 x64 Cross Tools Command Prompt" or similar, or run `vcvarsall.bat x86_amd64` or `setenv.cmd" /Release /x64`.

**Note**: Refer to the [compiler list](http://www.boost.org/build/doc/html/bbv2/reference/tools.html#bbv2.reference.tools.compilers) of the Boost documentation 
to find the version of the MSVC that you need to provide using the `toolset=` option.

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Use `dumpbin /headers <file>` to check whether a particular object file is 32 or 64-bit.

##### Examples

Build 32-bit Boost with Visual Studio 2013:

    > b2 toolset=msvc-12.0 address-model=32 link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

Build 64-bit Boost with Visual Studio 2015:

    > b2 toolset=msvc-14.0 address-model=64 link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

##### Side-by-side 32 and 64-bit versions

If you want to be able to have both the 32 and 64-bit versions of Boost available, add `--stagedir=./stage/x86_32` when building the 32-bit version and `--stagedir=./stage/x86_64` when building the 64-bit one, and they will be placed in `stage\x86_32\lib` and `stage\x86_64\lib` respectively. Later on you when building repositories that depend on this one, you will be able to point CMake the correct version of the libraries by using `-DBOOST_LIBRARYDIR="c:\boost\boost_1_xx_y\stage\x86_XX\lib`.

#### Ubuntu Linux

Install the required packages to build Boost:

    sudo apt-get install git make gcc g++

Additionally if you want to build non-native binaries (for example 32-bit binaries on a 64-bit Ubuntu installation):

    sudo apt-get install gcc-multilib

Open a terminal window and issue the following commands:

    $ cd $BOOST_ROOT
    $ ./bootstrap.sh
    $ ./b2 toolset=gcc cxxflags=-fPIC cflags=-fPIC address-model=<32,64> link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Use `objdump -f <file>` to check whether a particular object file is 32 or 64-bit.

##### Side-by-side 32 and 64-bit versions

If you want to be able to have both the 32 and 64-bit versions of Boost available, add `--stagedir=./stage/x86_32` when building the 32-bit version and `--stagedir=./stage/x86_64` when building the 64-bit one, and they will be placed in `stage/x86_32` and `stage/x86_64` respectively. Later on you when building repositories that depend on this one, you will be able to point CMake the correct version of the libraries by using `-DBOOST_LIBRARYDIR="~/boost/boost_1_xx_y/stage/x86_XX/lib`.

#### OS X 10.11 and later

Install Xcode from the App Store.

The simplest way to install Boost is to use Homebrew. If you don't have Homebrew installed simply run on a terminal:

    $ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you already have Homebrew installed, make sure it's up to date with:
  
    $ brew update
    $ brew upgrade

Once Homebrew is installed you can use the `brew` command on a terminal to install

    $ brew install boost --universal

This will download the boost source and compile it, so it might take a while.

