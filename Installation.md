# Installation

* [Supported environments](#Supported-environments)
    * [Operating system](#Operating-system)
    * [SoftDevice and IC](#SoftDevice-and-IC)
* [Installing driver](#Installing-driver)
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
* [Flashing connectivity HEX files](#Flashing-connectivity-HEX-files)
* [Known issues](#Known-issues)

---

## Supported environments

#### Operating system

* Windows (XP, 7, 8, 8.1, 10) 32 and 64-bit
* GNU/Linux (Ubuntu tested) 32 and 64-bit
* macOS (OS X) 32 and 64-bit

#### SoftDevice and IC

The libraries generated are compatible with the following SoftDevice API versions and nRF5x ICs:

* SoftDevice s130 API version 2: `s130_nrf51_2.x.x` (nRF51 and nRF52 series ICs)
* SoftDevice s132 API version 3: `s132_nrf52_3.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 5: `s132_nrf52_5.x.x` (only for nRF52 series ICs)
* SoftDevice s132 API version 6: `s132_nrf52_6.x.x` (only for nRF52 series ICs)
* SoftDevice s140 API version 6: `s140_nrf52_6.x.x` (only for nRF52 series ICs)

The HEX files in the `hex/sd_api_v<x>` folder include both the SoftDevice and the connectivity firmware which is required on the device to communicate with the `pc-ble-driver`.

##### [Back to top](#)
---

## Installing driver

This communication library works over any kind of serial port (UART), but it is most often used over a Segger J-Link USB CDC UART.
To set up the required J-Link drivers simply download and install the version matching you operating system:

* [SEGGER J-Link](https://www.segger.com/jlink-software.html)

After you have installed the required drivers and connected a J-Link enabled board (such as the Nordic Development Kits) the port should appear automatically.

In addition, you have to disable the `Mass Storage Device` in order to use `pc-ble-driver` to communicate with the device, [see `data corruption or drops issue` here](./issues/Issues#Data-corruption-or-drops).


#### Validating on Windows

The serial port will appear as `COMxx`.
Simply check the "Ports (COM & LPT)" section in the Device Manager.

#### Validating on Ubuntu Linux

The serial port will appear as `/dev/ttyACMx`.

**Note:** you need some extra steps to be able to use the device on Linux other than Windows and macOS.

By default the port is not accessible to all users. Type the command below to add your user to the `dialout` group to give it access to the serial port. Note that re-login is required for this to take effect.

```bash
    $ sudo usermod -a -G dialout <username>
```

To prevent the modemmanager service from trying to connect to the CDC ACM serial port:

```bash
    $ systemctl stop ModemManager.service
    $ systemctl disable ModemManager.service
```

#### Validating macOS (OS X)

The serial port will appear as `/dev/tty.usbmodemXXXX`.


> There is a known issue, check it [here](./issues/Issues.md#Timeout-error-related-to-the-SEGGER-J-Link-firmware)
> if you met any problems.

##### [Back to top](#)
---

## Installing tools

To flash the connectivity firmware you will need `nrfjprog` which is bundled with the nRF5x Command-Line Tools, which can be downloaded from:

* [nRF5x Command-Line Tools for Windows](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444)
* [nRF5x Command-Line Tools for Linux 32-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux32/52615)
* [nRF5x Command-Line Tools for Linux 64-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386)
* [nRF5x Command-Line Tools for OS X](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402)

> Add `nrfjprog` and `mergehex` to `PATH` on Linux and macOS.

##### [Back to top](#)
---

## Installing dependencies

To compile `pc-ble-driver` you will need the following tools:

* A C/C++ toolchain
* [Git](https://git-scm.com/) (>=2.19)
* [CMake](https://cmake.org/) (>=3.11)
* [vcpkg](https://github.com/Microsoft/vcpkg)

##### [Go to compile `pc-ble-driver` from source](#Compiling-pc-ble-driver-from-source)

To compile `connectivity` HEX files you will need additional tools:
* [Chocolatey](https://chocolatey.org/) (for installing GNU Make on Windows)
* [GNU Make](https://www.gnu.org/software/make/)
* [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
* [Python](https://www.python.org/)
* [pip](https://pypi.org/project/pip/)
* [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil)

##### [Go to compile `connectivity` HEX files](#Compiling-connectivity-hex-files)

> Follow the steps to install dependencies on a specific platform:

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
    SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
    ```

    Validate `Chocolatey` installation

    ```bash
    $ choco
    ```

2. Install `Git`.
    ```bash
    $ choco install -y git
    ```

3. Install `Cmake`.
    ```bash
    $ choco install -y cmake
    ```

4. Install [vcpkg](https://github.com/Microsoft/vcpkg).
    ```bash
    $ git clone https://github.com/Microsoft/vcpkg.git
    $ cd vcpkg
    $ .\bootstrap-vcpkg.bat
    ```

    Then add the vcpkg location to the `PATH` and set it as `VCPKG_ROOT` environment variable.

    And validate `vcpkg` installation
    ```bash
    $ vcpkg
    ```

> The following steps are needed only if you want to compile your own `connectivity` HEX files.

6. Install `make`.
    ```bash
    $ choco install -y make
    ```

7. Install `GNU Embedded Toolchain for Arm`
    ```bash
    $ choco install -y gcc-arm-embedded
    ```

    > Set its installation path as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.
    > By default:

    ```bash
    $ SET GCCARMEMB_TOOLCHAIN_PATH=C:\ProgramData\chocolatey\lib\gcc-arm-embedded\tools
    ```

8. Install `Python` and `pip`, and then install `nrfutil`
    ```bash
    $ pip install nrfutil

    # Validate installation
    $ nrfutil

    # Reboot if installation succeeds but validation fails
    ```

##### [Back to top](#)

#### Installing dependencies on Ubuntu Linux

1. Install `build-essential`.
    ```bash
    $ sudo apt-get -y install build-essential
    ```

2. Install `Git`
    ```bash
    $ sudo apt-get -y install git
    ```

    > If the installed version of `Git` is lower than required, then:
    ```bash
    $ sudo add-apt-repository ppa:git-core/ppa
    $ sudo apt update
    $ sudo apt install git
    ```

3. Install `Cmake`.
    ```bash
    $ sudo apt-get -y install cmake
    ```

    > Install `Cmake` from source if the version is lower than required.

4. Install [vcpkg](https://github.com/Microsoft/vcpkg).
    ```bash
    $ git clone https://github.com/Microsoft/vcpkg.git
    $ cd vcpkg
    $ ./bootstrap-vcpkg.sh
    ```

    Then add the vcpkg location to the `PATH` and `VCPKG_ROOT` environment variable.

    And validate `vcpkg` installation
    ```bash
    $ vcpkg
    ```

> The following steps are needed only if you want to compile your own `connectivity` HEX files.

5. Install `GNU Embedded Toolchain for Arm`.
    * Download from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * Extract
    * Set its location as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.

6. Install `Python` and `pip`, and then install `nrfutil`.
    ```bash
    $ pip install nrfutil

    # Validate installation
    $ nrfutil

    # Reboot if installation succeeds but validation fails
    ```

##### [Back to top](#)

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

    > Install `CMake` from source if the version is lower than required.

4. Install [vcpkg](https://github.com/Microsoft/vcpkg).
    ```bash
    $ git clone https://github.com/Microsoft/vcpkg.git
    $ cd vcpkg
    $ ./bootstrap-vcpkg.sh
    ```

    Then add the vcpkg location to the `PATH` and `VCPKG_ROOT` environment variable.

    And validate `vcpkg` installation
    ```bash
    $ vcpkg
    ```

> The following steps are needed only if you want to compile your own `connectivity` HEX files.

5. Install `GNU Embedded Toolchain for Arm`
    * Download from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * Extract
    * Set its location as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.

6. Install `Python` and `pip`, and then install `nrfutil`
    ```bash
    $ pip install nrfutil

    # Validate installation
    $ nrfutil

    # Reboot if installation succeeds but validation fails
    ```

##### [Back to top](#)
---

## Compiling pc-ble-driver from source

##### [Go to install dependencies](#Installing-dependencies) if you have not done that yet.

#### Compiling pc-ble-driver on Windows

1. Install vcpkg dependencies.

    ```bash
    # You are now in root directory of pc-ble-driver
    # Make sure %VCPKG_ROOT% is set and added to %PATH%
    $ mkdir build && cd build
    $ vcpkg install asio
    $ vcpkg install catch2
    ```

2. CMake

    To build 32-bit version with Visual Studio 2015:
    ```bash
    $ cmake -G "Visual Studio 14" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ..
    ```

    To build 64-bit version with Visual Studio 2015:

    ```bash
    $ cmake -G "Visual Studio 14 Win64" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ..
    ```

    > Change `-G "Visual Studio 14"` to `-G "Visual Studio 15"` if you are using Visual Studio 2017.

3. MSBuild

    ```bash
    $ msbuild ALL_BUILD.vcxproj
    ```

    > Optionally select the build configuration with the `/p:Configuration=` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available. For example:

    ```bash
    $ msbuild ALL_BUILD.vcxproj /p:Configuration=Debug
    ```

##### [Back to top](#)

#### Compiling pc-ble-driver on Ubuntu Linux or macOS

1. Install vcpkg dependencies.

    ```bash
    # You are now in root directory of pc-ble-driver
    # Make sure $VCPKG_ROOT is set and added to $PATH
    $ mkdir build && cd build
    $ vcpkg install asio
    $ vcpkg install catch2
    ```

2. CMake

    ```bash
    $ cmake \
        -G "Unix Makefiles" \
        -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
        ..
    ```

    > Optionally Select the build configuration with the `-DCMAKE_BUILD_TYPE` option. Typically `Debug`, `Release`, `MinSizeRel` and `RelWithDebInfo` are available.
    >
    > Optionally select the target architecture (32 or 64-bit) using the `-DARCH` option. The values can be `x86_32`, `x86_64`, `x86_32,x86_64`.
    > For example:

    ```bash
    $ cmake \
        -G "Unix Makefiles" \
        -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DARCH=x86_32,x86_64 \
        ..
    ```

3. Make
    ```bash
    $ make
    ```

##### [Back to top](#)
---

## Compiling connectivity HEX files

##### [Go to install dependencies](#Installing-dependencies) if you have not done that yet.

##### [Go to compile `pc-ble-driver` from source](#Compiling-pc-ble-driver-from-source) if you have not done that yet.

Two additional flags must be passed to CMake to create project files for connectivity firmware.

* `COMPILE_CONNECTIVITY=1`
* `CONNECTIVITY_VERSION=<version>`

> `COMPILE_CONNECTIVITY` is set to 1 to enable compiling connectivity firmware.
>
> `CONNECTIVITY_VERSION` defines a version for the compiled connectivity firmware.

The HEX files are available in the `hex/sd_api_v<x>` folder after compilation. They include the SoftDevice and the connectivity application.

Make sure the following environment variables are set:
* `VCPKG_ROOT`
* `GCCARMEMB_TOOLCHAIN_PATH`

Make sure the following paths have been added to PATH:
* `VCPKG_ROOT`
* `mergehex`

> Follow the steps to install dependencies on a specific platform:

#### Compiling connectivity HEX files on Windows

1. Check environment
    ```bash
    # You are now in root directory of pc-ble-driver
    $ SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
    $ cd hex
    $ mkdir build && cd build
    ```

2. CMake
    ```bash
    # Make sure environment variables have been set
    # as described at beginning of this section
    # Modify -DCONNECTIVITY_VERSION=a.b.c
    $ cmake -G "Visual Studio 14" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DCOMPILE_CONNECTIVITY=1 -DCONNECTIVITY_VERSION=1.0.0 ..
    ```
    Check more options at [compiling pc-ble-driver on Windows](#Compiling-pc-ble-driver-on-Windows)

3. MSBuild
    ```bash
    $ msbuild compile_connectivity.vcxproj
    ```

##### [Back to top](#)

#### Compiling connectivity HEX files on Ubuntu Linux or macOS

1. Check environment
    ```bash
    # You are now in root directory of pc-ble-driver
    $ cd hex
    $ mkdir build && cd build
    ```

2. CMake
    ```bash
    # Make sure environment variables have been set
    # as described at beginning of this section
    # Modify -DCONNECTIVITY_VERSION=a.b.c
    $ export TMP=/tmp
    $ cmake \
        -G "Unix Makefiles" \
        -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
        -DCOMPILE_CONNECTIVITY=1 \
        -DCONNECTIVITY_VERSION=1.0.0 \
        ..
    ```
    Check more options at [compiling pc-ble-driver on Ubuntu Linux or macOS](#Compiling-pc-ble-driver-on-Ubuntu-Linux-or-macOS)

3. Make
    ```bash
    $ make compile_connectivity
    ```

##### [Back to top](#)
---

## Flashing connectivity HEX files

[Go to install tools](#Installing-tools) if the nRF5x Command-Line Tools have not been installed yet.

To use this library you will need to flash the connectivity firmware on a nRF5x IC

Once you have installed the nRF5x Command-Line Tools, you can erase and program the IC:

    $ nrfjprog -f NRF5<x> -e
    $ nrfjprog -f NRF5<x> --program hex/sd_api_v<x>/connectivity_<ver>_<baudrate>_with_s<x>_<a>.<b>.<c>.hex

##### [Back to top](#)
---

## Known issues

When meeting problems during installing `pc-ble-driver`, see [Issues.md](./issues/Issues.md).


##### [Back to top](#)
---
