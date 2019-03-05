# Getting started with the examples

#### Building

Project files for `heart_rate_monitor` and `heart_rate_collector` examples are created and built together with `pc-ble-driver` by default. It is also possible to (re)build only the examples.

If there is no Makefile in `build/examples`, follow `Compiling pc-ble-driver from source` in [README.md](/README.md#Compiling-pc-ble-driver-from-source) to create it.

From project root
    $ cd build/examples/
    $ make

If everything went well, you should see:

    $ ls
    > heart_rate_monitor_v2 heart_rate_monitor_v3 ...

Where `heart_rate_monitor_v2` and `heart_rate_monitor_v3` etc are the executables you will run.

#### Running

Plug your nRF5 DK into your PC and [Program the connectivity firmware](/README.md#Programming-connectivity-HEX-files).

Note: the examples communicate with a baud rate of 1M [by default](https://github.com/NordicSemiconductor/pc-ble-driver/blob/master/examples/heart_rate_monitor/main.c#L420), so make sure you program your nRF5 device with `hex/sd_api_v<x>/connectivity_<ver>_1m_with_s<v>_<a>.<b>.<c>.hex` where `1m` corresponds to the baud rate of 1M.

If you are on macOS or a recent version of Linux there is a known [J-Link issue](https://github.com/NordicSemiconductor/pc-ble-driver#macos-os-x) that you need to do a workaround for.

Now find out which serial port your device is on:

    $ ls /dev/{tty,cu}.*  // On Windows simply check the "Ports (COM & LPT)" section in the Device Manager.
    > /dev/tty.usbmodem<xxxx>

And run the example:

    $ ./heart_rate_monitor_v<x> /dev/tty.usbmodem<xxxx>

#### Evaluating

When running the `heart_rate_monitor` example you should see:

    > Serial port used: /dev/tty.usbmodem<xxxx>
    > Warning: Successfully opened /dev/tty.usbmodem<xxxx>. Baud rate: 1M. Flow control: none. Parity: none.

    > Status: 6, message: Target Reset performed
    > Status: 7, message: Connection active
    > Services initiated
    > Characteristics initiated
    > Advertising data set
    > Started advertising

The nRF5 device is advertising as a BLE peripheral. You can scan for it and connect to it using nRF Connect desktop or mobile.

After connecting to the nRF5 device from nRF Connect, you should see:

    > Connected, connection handle 0x0000

From here you can play around more and experiment with `pc-ble-driver`.

When running the `heart_rate_collector` example, the nRF5 device is scanning as a BLE Master. You can experiment with this example more by programming a separate nRF5 DK with the BLE Peripheral heart rate monitor example in the nRF5 SDK `nRF5_SDK_ROOT_PATH/examples/ble_peripheral/ble_app_hrs` so the two devices can interact.
