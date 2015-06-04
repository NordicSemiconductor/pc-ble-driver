# nRF51 Bluetooth Smart GATT/GAP Driver


## Introduction
The nRF51 Bluetooth Smart GATT/GAP Driver (from now on called the Driver) consists of a standard C dynamic library that lets a PC application set up and interact with an nRF51 SoftDevice through API function calls. The library mirrors the nRF51 S130 SoftDevice API and makes it available to the PC application.

Commands sent from the library are encoded and sent over UART to the nRF51 chip. The connectivity application running on the nRF51 chip decodes the commands and feeds them to the SoftDevice. Command responses and events are encoded and sent from the nRF51 chip to the PC library where they are decoded.

The library makes it possible to create applications on a PC that, in code, can be similar to applications that are running on the nRF51 chip.

Having this kind of API gives the following benefits:

* Setting up a peer device for a Device Under Test (DUT) using a test script.
* Coherent BLE APIs on a PC and an nRF51 chip.
* Creating prototype and test applications for an nRF51 chip on a PC.
* Migrating code between an nRF51 chip and a PC.


## Disclaimer
No guarantees are given regarding stability or backward compatibility. Interfaces are subject to change.


## Usage

Important information on using the driver and driver examples.

**OSX:**
To be able to run the C and Python examples on OSX you have to set the DYLD_LIBRARY_PATH environment variable to the location of the driver.
You can do this by entering the following command: export DYLD_LIBRARY_PATH=/Users/nordic/nrf51-ble-driver_darwin_0.0.0/driver/lib.
If you do not want to do this manually in each terminal, you can add the command to your  ~/.bash_profile file.


## Build details

**WIN32:**
The Driver library has a standard C language interface with CDECL calling convention.
The Driver library has been compiled with MinGW GCC tool chain (i686-w64-mingw32-g++, gcc version 4.8.2).

**Linux (Ubuntu 14.04.2 LTS):**
The Driver library is compiled with the C/C++ compiler provided by the system. The Driver library requires that the Boost libraries packaged with the distribution are installed on the system.
Command to install boost:
sudo apt-get install boost1.54

**OSX:**
The Driver library is compiled with the C/C++ compiler provided by the system.

### Build from source code
The source code for the Driver is available on GitHub [???]. See [???] for instructions on how to build the Driver from source code.


## Software bundle contents
The following is included in the software bundle:

### For all platforms

* **Include files (.h)**
Header files with declarations of the functions and types of the library API.

* **Code examples**
Examples to demonstrate the use of the library. C examples are located directly under /examples, with project files for GCC and MSVC compilers. Examples for python bindings are located under /python/examples.

* **Python language bindings**
Bindings that make it possible to call the library from Python 2.7.

* **Hex files**
Precompiled connectivity hex files merged with SoftDevice hex file.

* **Documentation**
(This file.) Description of the library and how to set up the dependencies.

### For Windows

* **Dynamic library binary (.dll)**
Binary implementation of the Driver.

* **Static import library (.lib)**
Object library that points to the corresponding functions in the dynamic library. Used for compile time function resolving.

### For Linux
* **Dynamic library binary (.so)**
Binary implementation of the Driver.

### For OSX
* **Dynamic library binary (.dylib)**
Binary implementation of the Driver.


## Hardware setup

The Driver supports the following boards:
* **pca10028:** nRF51422 development kit (v1.0 or higher)
* **pca10031:** nRF51422 development dongle (v1.0 or higher)

In order for the Driver to operate, a supported board from the list above must be connected to the PC.

The necessary firmware to use this driver can be programmed by just copying connectivity_1M_with_s130_1.0.0.hex file onto the drive your board shows up as. The file contains both the connectivity firmware and SoftDevice.

On Windows you can also use **nRFgo Studio** or **nrfjprog** to program the HEX files.

If you wish to compile the connectivity firmware yourself, you can do that. The SoftDevice and connectivity firmware source code is available in the **nRF51 SDK** (separate download).


## Connectivity application

The pre-built connectivity HEX files are available in the 'hex' folder.

The connectivity HEX files are configured to use 115,200 bps (default setting in the Driver) and 1 Mbps. This baud rate must correspond with the serial port configuration in the Driver application.

The pre-built connectivity hex files have the UART pins and parity configured for communication through Segger JLink.

The S130 ble_connectivity project for the HEX file is available in the nRF51 SDK 8.1.0 (separate download).

To build a hex file for use with the Driver, choose the hci version of the connectivity project.

The following modifications are required for using the Driver when communicating through Segger JLink:

**Required modifications**:
* In ser_phy_config_conn_nrf51.h change UART pins to UART through USB:
  * #define SER_PHY_UART_RX RX_PIN_NUMBER
  * #define SER_PHY_UART_TX TX_PIN_NUMBER
  * #define SER_PHY_UART_CTS CTS_PIN_NUMBER
  * #define SER_PHY_UART_RTS RTS_PIN_NUMBER
* In ser_config.h turn off parity:
  * #define SER_PHY_UART_PARITY false
* Add preprocessor definition: HCI_LINK_CONTROL

**Optional modifications**:
* Baud rate can be changed in ser_config.h:
  * #define SER_PHY_UART_BAUDRATE UART_BAUDRATE_BAUDRATE_Baud1M


## Compiling

### WIN32

#### Compile time resolving
In order to use the header files and link against the library at compile time, a supported C or C++ compiler must be used:

* **MSVC (Microsoft Visual C compiler)**
Requires a static import library in order to resolve the function calls. A lib file import library is included in this release.

* **GCC/MinGW GCC**
Can link against the library automatically, which means that no import library is required.

C/C++ compiler settings:
* Add the Driver include folder to the compiler search path.
* Add the Driver binary folder to the linker search path.

#### Run time resolving
The library can be called from a number of other languages when function resolving is done at run time. As run time resolving is done differently in different languages, please refer to the corresponding language documentation.

### Linux/OSX
* No special measures are necessary.


## Verifying the setup
To test and verify that the setup and the library are working as expected, compile and run these examples:

* The advertising example: If everything works, the example will print "Started advertising". The nRF51 chip will advertise for about 3 minutes. The advertising packets can be discovered by using Master Control Panel or a similar program. The example code advertises with the name "Example".

* The multilink example: If everything works, the example will print "Scan started". The application will then print "Successfully connected to: 0x[device_address]" each time it finds a device advertising with the name "Example".


## API reference documentation
The SoftDevice API is documented in the nRF51 SDK documentation:
https://developer.nordicsemi.com/nRF51_SDK/nRF51_SDK_v8.x.x/doc/8.1.0/

The Driver extends the SoftDevice API with some extra functions which are currently documented in the header file sd_rpc.h and through the examples in this release.

The functions and events from the following header files are available:
* ble.h
* ble_gap.h
* ble_gattc.h
* ble_gatts.h
* ble_l2cap.h
* sd_rpc.h

Other API entries to note are the sd_ble_evt_get function and the ble_evt_tx_complete event. They are both implemented, but should not be used since they are handled in the connectivity firmware.


## Python bindings

Python bindings give the possibility to use the library from Python. See the included Python examples for demonstration on how to use the bindings.


## Included example projects

### Peripheral examples


#### Advertising
An example showing how to set up the nRF51 chip to start advertising. The application supports connection and service discovery in addition to advertising.

#### Heart rate monitor
Example implementation of the Heart Rate profile. When a central device connects to the peripheral and enables CCCD (Client Characteristic Configuration Descriptor), the peripheral starts sending notifications with simulated heart rate values.

### Central examples

#### Multilink
An example showing how to set up the nRF51 chip to start scanning and connecting to devices. The application supports connecting to multiple devices based on their advertised names.

#### Heart rate collector
Example implementation of a heart rate collector. Scans for and connects to a peripheral named "Nordic_HRM". After connecting you can enable the heart rate service CCCD (Client Characteristic Configuration Descriptor), after which notifications are sent from the peripheral until the CCCD is disabled.

### Concurrent central and peripheral example

#### Heart rate relay
An example demonstrating how to use the library to set up a concurrent central and peripheral operation. The example is a combination of a heart rate collector and monitor. The local central will connect to a peer heart rate monitor peripheral, and the local peripheral will relay the data to a peer central.


## Known issues

* Driver
  * Internal buffers in the Driver will sometimes overflow and give an error when packets arrive faster than the Driver can process them.
  * An issue related to the JLink internal firmware requires the development kit/dongle to be power cycled after programming the connectivity firmware, or else the UART might hang.

* Python bindings
  * Application sometimes freezes when termination is attempted.


## Release notes

### v0.5.0 (May 2015):
* Added S130 support.
* Removed S110, S120 support.
* Modified the directory structure in the package.
* Added a Heart Rate Relay example for demonstration of concurrent peripheral and central operation.
* Fixed an issue regarding synchronization of sequence numbers between nRF51 and BLE Driver.
* Source code released to GitHub: https://github.com/NordicSemiconductor/pc-ble-driver

### v0.4.1 (April 2015):
* Added support for OSX (10.9 or newer).
* Changed the default serial port speed back to 115,200 bps because of issues with OSX.

### v0.4.0 (April 2015):
* Added support for Ubuntu Linux 14.02.2 LTS (64-bit).
* Added SoftDevice and Connectivity firmware as one hex file programmable through the JLink storage device.
* Changed the default serial port speed from 115,200 bps to 1 Mbps.
* Various stability fixes.

### v0.3.0 (January 2015):
* Added support for S120.
* HEX files with higher baud rates.
* Added S120 example: multilink.
* Added S120 example: heart rate collector.
* Added python bindings S120 example: multilink.
* Added python bindings S120 example: heart rate collector.

### v0.2.0 (September 2014):
* Supports version 6.0.0 of SDK and version 7.0.0 of S110 SoftDevice.
* Changed API from asynchronous to synchronous function calls.

### v0.1.0 (April 2014):
* First public release.
* Added functions to configure serial port parameters.
* Added a function to specify a log file.
* Added a function to specify a log message filter.
* Added a function to close serial port and release resources.
* Added internal resource locking to avoid simultaneous calls to the library.
* Added time-out for the blocking wait function.
* Added a new example: heart rate monitor.
* Added msvc project files to c examples.
* Modified the directory structure, moved bindings examples to bindings directory.

### v0.0.2 (March 2014):
* Changed API from synchronous to asynchronous function calls.
* Modified library internals to use asynchronous serialization code base.
* Added Python bindings.
* Updated connectivity firmware hex files.

### v0.0.1 (February 2014):
* First pre-release.


## Licenses

The licenses for the Driver, the Driver bindings and the provided firmware are available in the licenses directory.
