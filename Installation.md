# Installation

* [Supported environments](#Supported-environments)
    * [Operating system](#Operating-system)
    * [SoftDevice and IC](#SoftDevice-and-IC)
* [Installing driver](#Installing-driver)
* [Installing tools](#Installing-tools)
* [Installing dependencies](#Prerequisites)
    * [Installing on Windows](#Prerequisites)
    * [Installing Ubuntu Linux](#Prerequisites)
    * [Installing macOS](#Prerequisites)
* [Compiling pc-ble-driver from source](#Prerequisites)
* [Compiling connectivity HEX files](#Prerequisites)
* [Flashing connectivity HEX files](#Prerequisites)
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

After you have installed the required drivers and connected a J-Link enabled board (such as the Nordic Development Kits) the port should appear automatically

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

> There is a known issue, check it [HERE](./issues/Issues.md#Timeout-error-related-to-the-SEGGER-J-Link-firmware)
> if you met any problems.

##### [Back to top](#)
---

## Installing tools

To flash the connectivity firmware you will need `nrfjprog` which is bundled with the nRF5x Command-Line Tools, which can be downloaded from:

* [nRF5x Command-Line Tools for Windows](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Win32/33444)
* [nRF5x Command-Line Tools for Linux 32-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux32/52615)
* [nRF5x Command-Line Tools for Linux 64-bit](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386)
* [nRF5x Command-Line Tools for OS X](https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-OSX/53402)

##### [Back to top](#)
---

## Installing dependencies

To compile `pc-ble-driver` you will need the following tools:

* Visual Studio 2015 or later (on Windows)
* A C/C++ toolchain (on Linux or macOS)
* [Git](https://git-scm.com/)
* [CMake](https://cmake.org/) (>=3.11)
* [vcpkg](https://github.com/Microsoft/vcpkg)

To compile `connectivity` HEX files you will need additional tools:
* [Chocolatey](https://chocolatey.org/) (for installing GNU Make on Windows)
* [GNU Make](https://www.gnu.org/software/make/)
* [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)

> Follow the steps to install dependencies on a specific platform:

#### Installing on Windows

1. Download `Visual Studio` and install.

2. Download `Cmake` from [here](https://cmake.org/download/) and install.

3. Install [vcpkg](https://github.com/Microsoft/vcpkg).
    ```bash
    $ git clone https://github.com/Microsoft/vcpkg.git
    $ cd vcpkg
    $ .\bootstrap-vcpkg.bat
    ```

    Then add the vcpkg location to the PATH environment variable.

    And validate `vcpkg` installation
    ```bash
    $ vcpkg
    ```

> The following steps are needed only if you want to compile your own `connectivity` HEX files.

4. Install [Chocolatey](https://chocolatey.org/install/)
    * Install with `cmd.exe` (Run as administrator), or
    ```bash
    # Copy everything below
    @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
    ```

    * Install with `PowerShell.exe` (Run as administrator)
    ```bash
    # Copy everything below
    Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    ```

    * Validate `Chocolatey` installation
    ```bash
    $ choco
    ```

5. Install `make` by using `Chocolatey`

    Run `cmd.exe` or `PowerShell.exe` as administrator.
    ```bash
    $ choco install make
    ```

    Validate `make` installation
    ```bash
    $ make
    ```

6. Install `GNU Embedded Toolchain for Arm`
    * Download from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * Install
    * Set its installation path as `GCCARMEMB_TOOLCHAIN_PATH` in environment variables.

#### Installing on Ubuntu Linux

#### Installing on macOS

##### [Back to top](#)
---

## Compiling the connectivity .hex files

CMake is used to build connectivity firmware. The .hex files are available in the `hex/sd_api_v<x>` folder after compilation. They include the SoftDevice and the connectivity application.

[Compiling pc-ble-driver from source](https://github.com/NordicSemiconductor/pc-ble-driver/blob/master/Installation.md#compiling-the-connectivity-hex-files)
### Dependencies
1. Follow instructions in `Compiling pc-ble-driver from source` and install all dependencies.
2. A recent version of GNU Make.
3. Install a recent version of [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads). And set its installation path as `GCCARMEMB_TOOLCHAIN_PATH`.

#### Windows
* Use chocolatey package manager for Windows to install Make.

1. Install [Chocolatey](https://chocolatey.org/).
2. Open cmd as `Administrator` and type `choco install make`.
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

## Flashing the connectivity firmware

To use this library you will need to flash the connectivity firmware on a nRF5x IC

Once you have installed the nRF5x Command-Line Tools, you can erase and program the IC:

    $ nrfjprog -f NRF5<x> -e
    $ nrfjprog -f NRF5<x> --program hex/sd_api_v<x>/connectivity_<ver>_<baudrate>_with_s<x>_<a>.<b>.<c>.hex

## Known issues

When meeting problems during installation `pc-ble-driver`, see [Issues.md](./issues/Issues.md).