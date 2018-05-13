## SoftDevice and IC support

The libraries generated are compatible with the following SoftDevice API versions and nRF5x ICs:

* SoftDevice s130 API version 2: `s130_nrf51_2.x.x` (nRF51 and nRF52 series ICs)
* SoftDevice s132 API version 5: `s132_nrf52_5.x.x` (only for nRF52 series ICs)

The .hex files included in the `hex/sd_api_v<x>` folder include both the SoftDevice and the connectivity firmware required to communicate with it.

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
    $ nrfjprog -f NRF5<x> --program hex/sd_api_v<x>/connectivity_<ver>_<baudrate>_with_s13<v>_<a>.<b>.<c>.hex

### J-Link USB CDC serial ports

After you have installed the required drivers and connected a J-Link enabled board (such as the Nordic Development Kits) the port should appear automatically

#### Windows

The serial port will appear as `COMxx`. Simply check the "Ports (COM & LPT)" section in the Device Manager.

#### Ubuntu Linux

The serial port will appear as `/dev/ttyACMx`. By default the port is not accessible to all users. Type the command below to add your user to the `dialout` group to give it access to the serial port. Note that re-login is required for this to take effect.

    sudo usermod -a -G dialout <username>

To prevent the modemmanager service from trying to connect to the CDC ACM serial port:

    systemctl stop ModemManager.service
    systemctl disable ModemManager.service

#### macOS (OS X)

The serial port will appear as `/dev/tty.usbmodemXXXX`.

**IMPORTANT NOTE**

On macOS (OS X) there is a known issue with the Segger J-Link firmware (that runs on the Debug probe on the board) related to USB packet sizes. This results in the timeout error `Failed to open nRF BLE Driver. Error code: 0x0D` when the serial port is attempted to be opened.

There are two ways to solve this issue:

1. Use the Segger firmware, but disable the Mass Storage Device (MSD) feature. Instructions are available [here](https://wiki.segger.com/index.php?title=J-Link-OB_SAM3U).

2. Replace the firmware on the Debug probe with the mbed DAPLink firmware: 
    - Enter bootloader mode by powering off the nRF5 Development Kit and then pressing IF BOOT/RESET while you power on the kit. 
    - Drag and drop the [nrf5x_osx_fix.bin](https://github.com/NordicSemiconductor/pc-ble-driver/blob/master/tools/nrf5x_osx_fix.bin) file into the BOOTLOADER mass storage device.

If you want to revert back to the Segger firmware you will have to download the it from [this location](http://www.nordicsemi.com/eng/nordic/Products/nRF51-DK/nRF5x-OB-JLink-IF/52276)

## Compiling the connectivity .hex files

Precompiled connectivity firmware are provided and can be used with standard Nordic Development Kits. The .hex files are available in the `hex/sd_api_v<x>` folder. They include the SoftDevice and the connectivity application.

You only need to recompile the connectivity application if you want to run it on a custom board. You can use the `hex/sd_api_v<x>/bootstrap_sd_api_v<X>.sh` script to download and patch the nRF SDK and the application with ease. Using this scripts, the steps 1 and 2 below are done automatically:

1. [Download the nRF SDK 11 or 14](https://developer.nordicsemi.com/nRF5_SDK/) (depending on the SoftDevice API you want to use) and unzip `nRF5_SDK_<x>.<y>.<z>_<sha>.zip`
2. Apply the patch `hex/sd_api_v<x>/SDK<ver>_connectivity.patch` from the unzipped SDK folder (e.g. `git apply -p1 --ignore-whitespace /repos/pc-ble-driver/hex/sd_api_v2/sdk110_connectivity.patch`)
3. Open the connectivity project `<sdk>/examples/ble_central_and_peripheral/ble_connectivity/pca100<xx>/ser_s13<x>_hci`
4. Compile it using the the compiler of your choice
5. Merge the built connectivity hex file with the corresponding SoftDevice hex file (e.g. `mergehex -m connectivity.hex softdevice.hex -o connectivity_with_softdevice.hex`)

## Building Boost

The Boost static libraries required by this drivers must be built before you can build any of the
repositories above that depend on pc-ble-driver.

### Obtain the Boost source code

Note: This step is not required for macOS (OS X).

Use the following link to download the Boost source code:

* [Boost](http://www.boost.org/users/download) (>=1.67.0)

- Download and extract Boost to a folder of your choice.
- Set the environment variable `BOOST_ROOT` to the path where you have extracted Boost.

For example on Windows assuming you've unpacked Boost in `c:\boost\boost_1_xx_y`:

    setx BOOST_ROOT "c:\boost\boost_1_xx_y"

And on Linux or macOS (OS X) assuming you've unpacked Boost in `~/boost/boost_1_xx_y`:

    export BOOST_ROOT="~/boost/boost_1_xx_y"

#### Windows 

Install Microsoft Visual Studio. The following versions supported are:

* Visual Studio 2015 (MSVC 14.0)

Open a Microsoft Visual Studio Command Prompt and issue the following commands:

    > cd %BOOST_ROOT%
    > bootstrap.bat
    > b2 toolset=msvc-<VV.V> address-model=<32,64> link=static --with-thread --with-system --with-regex --with-date_time --with-chrono

**Note**: If you intend to build a 64-bit version of Boost, you might need to open a 64-bit command prompt such as
"Visual Studio 2015 x86 x64 Cross Tools Command Prompt" or similar, or run `vcvarsall.bat x86_amd64` or `setenv.cmd" /Release /x64`.

**Note**: Refer to the [compiler list](http://www.boost.org/build/doc/html/bbv2/reference/tools.html#bbv2.reference.tools.compilers) of the Boost documentation 
to find the version of the MSVC that you need to provide using the `toolset=` option.

**Note**: Select 32 or 64-bit with the `address-model=` option.

**Note**: Use `dumpbin /headers <file>` to check whether a particular object file is 32 or 64-bit.

##### Examples

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

#### macOS (OS X) 10.11 and later

Install Xcode from the App Store.

The simplest way to install Boost is to use Homebrew. If you don't have Homebrew installed simply run on a terminal:

    $ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you already have Homebrew installed, make sure it's up to date with:
  
    $ brew update
    $ brew upgrade

Once Homebrew is installed you can use the `brew` command on a terminal to install boost:

    $ brew install boost

This will download the boost source and compile it, so it might take a while.

## Compiling pc-ble-driver from source

Assuming that you have built the Boost libraries and installed the tools required to do so [as described above](#building-boost), you can now build the shared library.

### Dependencies

To build this project you will need the following tools:

* [CMake](https://cmake.org/) (>=3.11)
* A C/C++ toolchain (should already have been installed to build Boost)

See the following sections for platform-specific instructions on the installation of the dependencies.

#### Windows 

* Install the latest CMake stable release by downloading the Windows Installer from:

[CMake Downloads](https://cmake.org/download/)

Open a Microsoft Visual Studio Command Prompt and issue the following from the root folder of the repository:

    > cd build
    > cmake -G "Visual Studio 14 <Win64>" <-DBOOST_LIBRARYDIR="<Boost libs path>>" ..
    > msbuild ALL_BUILD.vcxproj </p:Configuration=<CFG>>

**Note**: Add `Win64` to the `-G` option to build a 64-bit version of the driver.

**Note**: Optionally select the location of the Boost libraries with the `-DBOOST_LIBRARYDIR` option.

**Note**: Optionally select the build configuration with the `/p:Configuration=` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.

##### Examples

Building for with 64-bit Visual Studio 2015:

    > cmake -G "Visual Studio 14" ..

#### Ubuntu Linux

Install cmake:

    $ sudo apt-get install cmake

Then change to the root folder of the repository and issue the following commands:

    $ cd build
    > cmake -G "Unix Makefiles" <-DCMAKE_BUILD_TYPE=<build_type>> <-DARCH=<x86_32,x86_64>> <-DBOOST_LIBRARYDIR="<Boost libs path>>" ..
    $ make

**Note**: Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.

**Note**: Optionally select the target architecture (32 or 64-bit) using the `-DARCH` option.

**Note**: Optionally select the location of the Boost libraries with the `-DBOOST_LIBRARYDIR` option.

#### macOS (OS X) 10.11 and later

Install cmake with Homebrew with the `brew` command on a terminal:

    $ brew install cmake

Then change to the root folder of the repository and issue the following commands:

    $ cd build
    $ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE= <build_type> ..
    $ make

**Note**: Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.
