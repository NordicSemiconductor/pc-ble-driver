# pc-ble-driver

`pc-ble-driver` provides C/C++ libraries for Bluetooth Low Energy nRF5 SoftDevice serialization.

## Overview
`pc-ble-driver` consists of a set of static and shared libraries that provide [SoftDevice](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.softdevices52/dita/nrf52/softdevices.html?cp=2_3) functionality to the application via serial port communication with an nRF5 connectivity chip running the SoftDevice and connectivity software, included as a single .hex file [here](./hex/). For more information on SoftDevice serialization see [Serialization](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.0.0/lib_serialization.html?cp=4_0_0_3_33).

The C/C++ libraries can be interfaced with directly, but are also provided as higher-level bindings that ease development at the cost of reduced control (acceptable in most cases):

* [pc-ble-driver-js Node.JS bindings](https://github.com/NordicSemiconductor/pc-ble-driver-js)
* [pc-ble-driver-py Python bindings](https://github.com/NordicSemiconductor/pc-ble-driver-py)

## Installation

For detailed guidelines on building and installing `pc-ble-driver` and its dependencies see [Installation.md](./Installation.md).

## Examples

The [examples](./examples) serve as a great starting point for development with `pc-ble-driver`. Examples include a [heart rate monitor](./examples/heart_rate_monitor/) (BLE peripheral) and [heart rate collector](./examples/heart_rate_collector/) (BLE master) and show the basic structure of an application built on `pc-ble-driver`.

Now that you have successfully built and installed `pc-ble-driver`, you are ready to run the examples in `pc-ble-driver/examples/`. First verify that the static and shared libraries exist in the directory the examples expect them to be in.

    $ cd pc-ble-driver/
    $ ls build/
    > libpc_ble_driver_static_sd_api_v2.a libpc_ble_driver_shared_sd_api_v2.dylib libpc_ble_driver_static_sd_api_v5.a  libpc_ble_driver_shared_sd_api_v5.dylib test_uart ...

To quickly get the examples up and running, see [examples/README.md](./examples/README.md).

## Architecture

![Architecture](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.0.0/architecture_overview_serialization.svg)

Where the 'Application chip' is just generic hardware (i.e. a Windows, OS X or Linux device), although it could also be an Arduino or Raspberry Pi for example.

## License

See the [license file](./LICENSE) for details.
