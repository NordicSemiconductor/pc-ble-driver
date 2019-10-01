# pc-ble-driver

`pc-ble-driver` provides C/C++ libraries for Bluetooth Low Energy nRF5 SoftDevice serialization.

* [Overview](#Overview)
* [Architecture](#Architecture)
* [Supported environments](#Supported-environments)
    * [Operating system](#Operating-system)
    * [SoftDevice and IC](#SoftDevice-and-IC)
* [Installing device drivers](#Installing-device-drivers)
    * [Driver installation](#Driver-installation)
    * [Driver validation](#Driver-validation)
* [Installing tools](#Installing-tools)
* [Installing dependencies](#Installing-dependencies)
    * [Installing on Windows](#Installing-dependencies-on-Windows)
    * [Installing on Ubuntu Linux](#Installing-dependencies-on-Ubuntu-Linux)
    * [Installing on macOS](#Installing-dependencies-on-macOS)
* [Compiling pc-ble-driver from source](#Compiling-pc-ble-driver-from-source)
    * [Compiling on Windows](#Compiling-pc-ble-driver-on-Windows)
    * [Compiling on Ubuntu Linux or macOS](#Compiling-pc-ble-driver-on-Ubuntu-Linux-or-macOS)
* [Compiling connectivity HEX files](#Compiling-connectivity-HEX-files)
    * [Compiling on Windows](#Compiling-connectivity-HEX-files-on-Windows)
    * [Compiling on Ubuntu Linux or macOS](#Compiling-connectivity-HEX-files-on-Ubuntu-Linux-or-macOS)
* [Programming connectivity HEX files](#Programming-connectivity-HEX-files)
* [Examples](#Examples)
* [Known issues](#Known-issues)
* [License](#License)

---

## Overview

`pc-ble-driver` consists of a set of static and shared libraries that provide [SoftDevice](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.s140.api.v6.1.1/modules.html) functionality to the application via serial port communication with an nRF5 connectivity chip running the SoftDevice and connectivity software, included as a single .hex file [here](./hex/). For more information on SoftDevice serialization see [Serialization](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.3.0/lib_serialization.html).

The C/C++ libraries can be interfaced with directly, but are also provided as higher-level bindings that ease development at the cost of reduced control (acceptable in most cases):

* [pc-ble-driver-js Node.JS bindings](https://github.com/NordicSemiconductor/pc-ble-driver-js)
* [pc-ble-driver-py Python bindings](https://github.com/NordicSemiconductor/pc-ble-driver-py)

---


## Architecture

![Architecture](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.3.0/architecture_overview_serialization.svg)

Where the 'Application chip' is just generic hardware (i.e. a Windows, Linux or macOS device), although it could also be an Arduino or Raspberry Pi for example.

---

## Supported environments

#### Operating system

* Windows 7, 8 and 10, 32 and 64-bit (tested on Windows 7)
* Ubuntu Linux LTS 64-bit (tested on Ubuntu 18.04)
* macOS 64-bit (tested on 10.14 Mojave)

#### SoftDevice and IC

To use pc-ble-driver, your Development Kit needs to have the correct firmware. The needed firmwares are found in the `hex/sd_api_v<x>` folder and contain the SoftDevice and connectivity firmware required to communicate with `pc-ble-driver`.

The generated libraries are compatible with the following SoftDevice API versions and nRF5x ICs:

* SoftDevice s130 API version 2: `connectivity_<version>_1m_with_s130_2.x.x` (nRF51 and nRF52 series ICs)
* SoftDevice s132 API version 3: `connectivity_<version>_<1m|*usb>_with_s132_3.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 5: `connectivity_<version>_<1m|*usb>_with_s132_5.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 6: `connectivity_<version>_<1m|*usb>_with_s132_6.x.x` (only for nRF52 series ICs)
* SoftDevice s140 API version 6: `connectivity_<version>_<1m|*usb>_with_s140_6.x.x` (only for nRF52 series ICs)

*usb) only for nRF52 series ICs with USBD peripheral

###### Supported Development Kits
| PCA      | Official name                | Article number | Notes    |
-----------|------------------------------|----------------|----------|
| PCA10028 | nRF51 DEVELOPMENT KIT        | nRF6824        |          |
| PCA10031 | nRF51 DONGLE                 | nRF6825        |          |
| PCA10040 | nRF52 DEVELOPMENT KIT        | nRF6827        |          |
| PCA10056 | nRF52840 { Development Kit } | nRF6828        | *)       |
| PCA10059 | nRF52840 { Dongle }          | nRF6829        | Can only use connectivity firmware with Nordic USB CDC serial port support  |

*) Can use both Nordic USB CDC serial port version and SEGGER J-Link-OB (VCOM) version. Using Nordic USB CDC serial port version on PCA10056 requires that you connect pins P0.18 and P0.24. The pins to QSPI chip must also be in place (they are by default). The algorithm for detecting if it is PCA10056 or PCA10059 is to check if it is possible to communicate with the QSPI chip. PCA10059 does not have a QSPI chip. The detection is used by nRF Connect DFU trigger to determine what pin to use for resetting the device when changing between DFU and application mode.

---

## Installing device drivers

### Driver installation

This communication library works over any kind of serial port (UART), but it is most often used over a Segger J-Link USB CDC UART.
To set up the required J-Link drivers simply download and install the version matching you operating system:

* [SEGGER J-Link](https://www.segger.com/jlink-software.html)

After you have installed the required drivers and connected a J-Link enabled board (such as the Nordic Development Kits) the port should be available.

In addition, you have to disable the `Mass Storage Device` in order to use `pc-ble-driver` to communicate with the device, [see `data corruption or drops issue`](./Issues.md#Data-corruption-or-drops).

### Driver validation

#### Validating on Windows

The serial port will appear as `COMxx`.
Simply check the "Ports (COM & LPT)" section in the Device Manager.

#### Validating on Ubuntu Linux

The serial port will appear as `/dev/ttyACMx`.

By default the port is not accessible to all users. Type the command below to add your user to the `dialout` group to give it access to the serial port. Note that re-login is required for this to take effect.

```bash
    $ sudo usermod -a -G dialout <username>
```

To prevent the modemmanager service from trying to connect to the CDC ACM serial port:

```bash
    $ systemctl stop ModemManager.service
    $ systemctl disable ModemManager.service
```

#### Validating on macOS

The serial port will appear as `/dev/tty.usbmodemXXXX`.


There is a known issue, check it [here](./issues/Issues.md#Timeout-error-related-to-the-SEGGER-J-Link-firmware)
if you met any problems.

---

## Installing tools

#### nRF5x Command-Line Tools

To program the connectivity firmware you will need `nrfjprog` which is bundled with the nRF5x Command-Line Tools, which can be downloaded from:

* [nRF5x Command-Line Tools for Windows](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444)
* [nRF5x Command-Line Tools for Linux 32-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux32/52615)
* [nRF5x Command-Line Tools for Linux 64-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386)
* [nRF5x Command-Line Tools for macOS](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402)

Add `nrfjprog` and `mergehex` to `PATH` on Linux and macOS.

#### nRF Connect Programmer (optional)

Alternatively, `nRF Connect Programmer` can help you to program the connectivty firmware with UI support.

Download
[nRF Connect Desktop]( https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF-Connect-for-Desktop)
and install `nRF Connect Programmer` there.

---

## Installing dependencies

To compile `pc-ble-driver` you will need the following tools:

* A C/C++ toolchain
* [Git](https://git-scm.com/) (>=2.19)
* [CMake](https://cmake.org/) (>=3.11)
* [vcpkg](https://github.com/NordicPlayground/vcpkg.git)

##### [Go to compile `pc-ble-driver` from source](#Compiling-pc-ble-driver-from-source)

To compile `connectivity` HEX files you will need additional tools:
* [Chocolatey](https://chocolatey.org/)
* [Ninja](https://ninja-build.org/)
* [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
* [Python](https://www.python.org/)
* [pip](https://pypi.org/project/pip/)
* [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil)

##### [Go to compile `connectivity` HEX files](#Compiling-connectivity-hex-files)

Follow the steps to install dependencies on a specific platform:

#### Installing dependencies on Windows

1. Download `Visual Studio 15` or later version and install.

2. Install [Chocolatey](https://chocolatey.org/install/).
    Install with `cmd.exe` (Run as administrator)
    ```bash
    # Copy everything below
    @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
    ```

    If `Chocolatey` has already been installed as described above but has not been added to PATH, run:
    ```bash
    $ SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
    ```

2. Install `Git`.
    ```bash
    $ choco install -y git
    ```

3. Install `CMake`.
    ```bash
    $ choco install -y cmake
    ```

4. Install [vcpkg](https://github.com/NordicPlayground/vcpkg.git).
    ```bash
    $ git clone https://github.com/NordicPlayground/vcpkg.git
    $ cd vcpkg
    $ git checkout fix/temporary-fix-spdlog-until-vcpkg-release # Temporary workaround for spdlog issue
    $ .\bootstrap-vcpkg.bat
    ```

    Then add the vcpkg location to the `PATH` and set it as `VCPKG_ROOT` environment variable.

The following steps are needed only if you want to compile your own `connectivity` HEX files.

1. Install `ninja`.
    ```bash
    $ choco install -y ninja
    ```

2. Download and install `GNU Embedded Toolchain for Arm` version 7-2018q2

    Download from [this](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) location.

    Follow the install instructions.

    Set its installation path as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.
    For example:

    ```bash
    $ set GCCARMEMB_TOOLCHAIN_PATH=c:\gccarmemb
    ```

3. Install `Python` and `pip`, and then install `nrfutil`
    ```bash
    $ pip install nrfutil

    # Reboot if installation succeeds but validation fails
    ```


#### Installing dependencies on Ubuntu Linux

1. Install `build-essential`.
    ```bash
    $ sudo apt-get -y install build-essential
    ```

2. Install `Git`
    ```bash
    $ sudo apt-get -y install git
    ```

    If the installed version of `Git` is lower than required, then:
    ```bash
    $ sudo add-apt-repository ppa:git-core/ppa
    $ sudo apt update
    $ sudo apt install git
    ```

3. Install `CMake`.
    ```bash
    $ sudo apt-get -y install cmake
    ```

    Install `CMake` from source if the version is lower than required.

4. Install [vcpkg](https://github.com/NordicPlayground/vcpkg/).
    ```bash
    $ git clone https://github.com/NordicPlayground/vcpkg.git
    $ cd vcpkg
    $ git checkout fix/temporary-fix-spdlog-until-vcpkg-release # Temporary workaround for spdlog issue
    $ ./bootstrap-vcpkg.sh
    ```

    Then add the vcpkg location to the `PATH` and `VCPKG_ROOT` environment variable.

The following steps are needed only if you want to compile your own `connectivity` HEX files.

1. Install `GNU Embedded Toolchain for Arm` version 7-2018q2.
    * Download from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * Extract
    * Set its location as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.

2. Install `Python` and `pip`, and then install `nrfutil`.
    ```bash
    $ pip install nrfutil

    # Reboot if installation succeeds but validation fails
    ```

#### Installing dependencies on macOS

1. Install `Xcode (>=10.1)`.

2. Install `gcc6` using  [HomeBrew](https://brew.sh/).
    ```bash
    $ brew install gcc6
    ```

3. Install `CMake` using [HomeBrew](https://brew.sh/).
    ```bash
    $ brew install cmake
    $ brew upgrade cmake
    ```

    Install `CMake` from source if the version is lower than required.

4. Install [vcpkg](https://github.com/NordicPlayground/vcpkg/).
    ```bash
    $ git clone https://github.com/NordicPlayground/vcpkg/
    $ cd vcpkg
    $ git checkout fix/temporary-fix-spdlog-until-vcpkg-release # Temporary workaround for spdlog issue
    $ ./bootstrap-vcpkg.sh
    ```

    Then add the vcpkg location to the `PATH` and `VCPKG_ROOT` environment variable.

 The following steps are needed only if you want to compile your own `connectivity` HEX files.

1. Install `GNU Embedded Toolchain for Arm` version 7-2018q2.
    * Download from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * Extract
    * Set its location as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.

2. Install `Python` and `pip`, and then install `nrfutil`
    ```bash
    $ pip install nrfutil

    # Reboot if installation succeeds but validation fails
    ```

---

## Compiling pc-ble-driver from source

##### [Go to install dependencies](#Installing-dependencies) if you have not done that yet.

#### Compiling pc-ble-driver on Windows

1. Install vcpkg dependencies.

    ```bash
    # cd <pc-ble-driver-root-folder>

    # Make sure %VCPKG_ROOT% is set and added to %PATH%
    $ mkdir build && cd build
    $ vcpkg install asio
    $ vcpkg install catch2
    $ vcpkg install --head spdlog
    ```

2. CMake
    Select the Visual Studio compiler to use according to this article: [Build C/C++ code on the command line](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2015)

    ```bash
    $ cmake -G Ninja ..
    ```

3. Compile

    ```bash
    $ cmake --build .
    ```

    Optionally select the build configuration with the `--config` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available. For example:

    ```bash
    $ cmake --build . --config Debug
    ```

#### Compiling pc-ble-driver on Ubuntu Linux or macOS

1. Install vcpkg dependencies.

    ```bash
    # cd <pc-ble-driver-root-folder>

    # Make sure $VCPKG_ROOT is set and added to $PATH
    $ mkdir build && cd build
    $ vcpkg install asio
    $ vcpkg install catch2
    $ vcpkg install --head spdlog
    ```

2. CMake

    ```bash
    $ cmake \
        -G Ninja \
        ..
    ```

    Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.

    Optionally select the target architecture (32 or 64-bit) using the `-DARCH` option. The values can be `x86_32`, `x86_64`, `x86_32,x86_64`.
    For example:

    ```bash
    $ cmake \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DARCH=x86_32,x86_64 \
        ..
    ```

3. Compile
    ```bash
    $ cmake --build .
    ```

---

## Compiling connectivity HEX files

##### [Go to install dependencies](#Installing-dependencies) if you have not done that yet.

##### [Go to compile `pc-ble-driver` from source](#Compiling-pc-ble-driver-from-source) if you have not done that yet.

Make sure the following environment variables are set:
* `VCPKG_ROOT`
* `GCCARMEMB_TOOLCHAIN_PATH`

Make sure the following paths have been added to PATH:
* `VCPKG_ROOT`
* `mergehex`

Follow the steps to install dependencies on a specific platform:

#### Compiling connectivity HEX files on Windows

1. Set environment
    ```bash
    # cd <pc-ble-driver-root-folder>
    $ SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

    # Make sure environment variables have been set
    # as described at beginning of this section
    ```

2. CMake
    ```bash
    $ mkdir build && cd build

    # Modify -DCONNECTIVITY_VERSION=a.b.c
    $ cmake -G Ninja -DCOMPILE_CONNECTIVITY=1 -DCONNECTIVITY_VERSION=1.0.0 ..
    ```

    `COMPILE_CONNECTIVITY` is set to 1 to enable compiling connectivity firmware.

    `CONNECTIVITY_VERSION` defines a version for the compiled connectivity firmware.

    Check more options at [compiling pc-ble-driver on Windows](#Compiling-pc-ble-driver-on-Windows)

3. Compile
    ```bash
    $ cmake --build . --target compile_connectivity
    ```

    The HEX files are available in the `hex/sd_api_v<x>` folder after compilation. They include the SoftDevice and the connectivity application.

#### Compiling connectivity HEX files on Ubuntu Linux or macOS

1. Set environment
    ```bash
    # cd <pc-ble-driver-root-folder>
    $ export TMP=/tmp

    # Make sure environment variables have been set
    # as described at beginning of this section
    ```

2. CMake
    ```bash
    $ cd hex
    $ mkdir build && cd build

    # Modify -DCONNECTIVITY_VERSION=a.b.c
    $ cmake \
        -G Ninja \
        -DCOMPILE_CONNECTIVITY=1 \
        -DCONNECTIVITY_VERSION=1.0.0 \
        ..
    ```

    `COMPILE_CONNECTIVITY` is set to 1 to enable compiling connectivity firmware.

    `CONNECTIVITY_VERSION` defines a version for the compiled connectivity firmware.

    Check more options at [compiling pc-ble-driver on Ubuntu Linux or macOS](#Compiling-pc-ble-driver-on-Ubuntu-Linux-or-macOS)

3. Compile
    ```bash
    $ cmake --build . --target compile_connectivity
    ```

    The HEX files are available in the `hex/sd_api_v<x>` folder after compilation. They include the SoftDevice and the connectivity application.

---

## Programming connectivity HEX files

### SEGGER J-Link-OB based kits
[Go to install tools](#Installing-tools) if the nRF5x Command-Line Tools have not been installed yet.

To use this library you will need to program the connectivity firmware on a nRF5x IC

Use [nRF5x Command-Line Tools](#Installing-tools) to erase and program the IC:

    $ nrfjprog -f NRF5<x> -e
    $ nrfjprog -f NRF5<x> --program hex/sd_api_v<x>/connectivity_<ver>_<baudrate>_with_s<x>_<a>.<b>.<c>.hex

Alternatively, use [nRF Connect Programmer](https://github.com/NordicSemiconductor/pc-nrfconnect-programmer)
to erase and program the IC.

### Nordic USB based kits

#### Installing drivers and tools
Some kits, like the pca10059 nRF52 dongle, do not have onboard debugger and will have to be programmed via serial DFU.
On Windows, device drivers are required for the kits to be detected correctly by the operating system. To install the required drivers, please make sure you have the latest [nRF Connect for Desktop](http://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF-Connect-for-desktop) installed.

#### Programming the connectivity firmware via serial DFU

Programming the connectivity firmware via serial DFU can be done from command line or from nRF Connect for Desktop.

##### Programming from command line with nrfutil

Device Firmware Upgrade with [nrfutil](https://infocenter.nordicsemi.com/topic/ug_nrfutil/UG/nrfutil/nrfutil_dfu_serial_usb.html) is normally done in two steps: 1: generating the DFU zip package, and 2: performing the DFU procedure. A DFU zip package has been pre-made and is included in this repository. To run the DFU procedure with nrfutil with the pre-made DFU package:

    nrfutil dfu usb_serial -pkg connectivity_x.x.x_usb_with_s<x>_<a>.<b>.<c>_dfu_pkg.zip -p <serial port>

---

## Examples

The [examples](./examples) serve as a great starting point for development with `pc-ble-driver`. Examples include a [heart rate monitor](./examples/heart_rate_monitor/) (BLE peripheral) and [heart rate collector](./examples/heart_rate_collector/) (BLE master) and show the basic structure of an application built on `pc-ble-driver`.

To quickly get the examples up and running, see [examples/README.md](./examples/README.md).

## Known issues

If you meet problems during installation of pc-ble-driver, please see [Issues.md](./Issues.md).

## Contributions

Feel free to propose changes by creating a pull request.

If you plan to make any non-trivial changes, please start out small and ask seek an agreement before putting too much work in it.
A pull request can be declined if it does not fit well within the current product roadmap.

In order to accept your pull request, we need you to sign our Contributor License Agreement (CLA). You will see instructions for doing this after having submitted your first pull request.

## Feedback

If you find any bugs, or have questions or other feedback in general, please submit a post on the [Nordic DevZone](https://devzone.nordicsemi.com) portal.
Note that bug reports should describe in sufficient detail how to reproduce the bug.

## License

See the [license file](./LICENSE) for details.
