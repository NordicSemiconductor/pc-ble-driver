## SoftDevice and IC support

The libraries generated are compatible with the following SoftDevice API versions and nRF5x ICs:

* SoftDevice s130 API version 2: `s130_nrf51_2.x.x` (nRF51 and nRF52 series ICs)
* SoftDevice s132 API version 3: `s132_nrf52_3.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 5: `s132_nrf52_5.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 6: `s132_nrf52_6.x.x` (only for nRF52 series ICs)
* SoftDevice s140 API version 6: `s140_nrf52_6.x.x` (only for nRF52 series ICs)

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
    $ nrfjprog -f NRF5<x> --program hex/sd_api_v<x>/connectivity_<ver>_<baudrate>_with_s<x>_<a>.<b>.<c>.hex

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

CMake is used to build connectivity firmware. The .hex files are available in the `hex/sd_api_v<x>` folder after compilation. They include the SoftDevice and the connectivity application.

[Compiling pc-ble-driver from source](https://github.com/NordicSemiconductor/pc-ble-driver/blob/master/Installation.md#compiling-the-connectivity-hex-files)
### Dependencies
1. Follow instructions in `Compiling pc-ble-driver from source` and install all dependencies.
2. Install Make.
3. Install [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads).

#### Windows
* Use chocolatey package manager for Windows to install Make.

1. Install [Chocolatey](https://chocolatey.org/).
2. Open cmd and type `choco install make`.
3. Make sure that make is exported in path variable.

### Compilation
Follow steps in `Compiling pc-ble-driver from source` for details on how to create project files for your platform. Two additional flags must be passed to CMake to create project files for connectivity firmware. `CONNECTIVITY_VERSION` defines a version for the compiled connectivity firmware.

* `COMPILE_CONNECTIVITY=1`
* `CONNECTIVITY_VERSION=<version>`

#### Example

Compiling on Linux

    $ cd build
    $ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -DCOMPILE_CONNECTIVITY=1 -DCONNECTIVITY_VERSION=1.0.0 ..
    $ cd hex
    $ make compile_connectivity

## Compiling pc-ble-driver from source

### Dependencies

To build this project you will need the following tools:

* [CMake](https://cmake.org/) (>=3.11)
* A C/C++ toolchain
* [vcpkg](https://github.com/Microsoft/vcpkg)

Install vcpkg as described [here](https://github.com/Microsoft/vcpkg).

Add the vcpkg location to the PATH environment variable.

See the following sections for platform-specific instructions on the installation of the dependencies.

#### Windows 

* Install the latest CMake stable release by downloading the Windows Installer from:

[CMake Downloads](https://cmake.org/download/)

Open a Microsoft Visual Studio Command Prompt and issue the following from the root folder of the repository:

    > vcpkg install asio
    > vcpkg install catch2
    > cd build
    > cmake -G "Visual Studio 14 <Win64>" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ..
    > msbuild ALL_BUILD.vcxproj </p:Configuration=<CFG>>

**Note**: Add `Win64` to the `-G` option to build a 64-bit version of the driver.

**Note**: Optionally select the build configuration with the `/p:Configuration=` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.

##### Examples

Building for with 64-bit Visual Studio 2015:

    > cmake -G "Visual Studio 14" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ..

#### Ubuntu Linux

Install cmake:

    $ sudo apt-get install cmake

Then change to the root folder of the repository and issue the following commands:

    $ cd build
    $ vcpkg install asio
    $ vcpkg install catch2
    $ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake <-DCMAKE_BUILD_TYPE=<build_type>> <-DARCH=<x86_32,x86_64>>" ..
    $ make

**Note**: Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.

**Note**: Optionally select the target architecture (32 or 64-bit) using the `-DARCH` option.

#### macOS (OS X) 10.11 and later

Install cmake with Homebrew with the `brew` command on a terminal:

    $ brew install cmake

Then change to the root folder of the repository and issue the following commands:

    $ vcpkg install asio
    $ vcpkg install catch2
    $ cd build
    $ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE= <build_type> ..
    $ make

**Note**: Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.
