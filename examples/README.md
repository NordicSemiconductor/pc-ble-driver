# Getting started with the examples

#### Building

Build the examples just like you built `pc-ble-driver`.

We will use `heart_rate_monitor` as the concrete example to follow along with, but building and running `hear_rate_collector` is exactly the same.

    $ cd pc-ble-driver/examples/heart_rate_monitor/
    $ mkdir build/
    $ cd build/

Note: this step depends on your operating system [as documented here](https://github.com/NordicSemiconductor/pc-ble-driver#compiling-pc-ble-driver-from-source).
    
    $ cmake .. -G "Unix Makefiles"
    $ make

If everything went well, you should see:

    $ ls pc-ble-driver/examples/heart_rate_monitor/build/
    > hrm_v2 hrm_v3 ...

Where `hrm_v2` and `hrm_v3` are the executables you will run.

#### Running

Plug your nRF5 DK into you're PC and [Flash the connectivity firmware](https://github.com/NordicSemiconductor/pc-ble-driver#flashing-the-connectivity-firmware).

Note: the examples communicate with a baud rate of 115200 [by default](https://github.com/NordicSemiconductor/pc-ble-driver/blob/master/examples/heart_rate_monitor/main.c#L420), so make sure you flash your nRF5 device with `sd_api_v<x>/connectivity_<ver>_115k2_with_s13<v>_<a>.<b>.<c>.hex` where `115k2` corresponds to the baud rate of 115200.

If you are on OS X there is a known [J-Link issue](https://github.com/NordicSemiconductor/pc-ble-driver#macos-os-x) that you need to do a workaround for.

Now find out which serial port your device is on:

    $ ls /dev/{tty,cu}.*  // On Windows simply check the "Ports (COM & LPT)" section in the Device Manager.
    > /dev/tty.usbmodem<xxxx>

And run the example:

    $ ./hrm_v<x> /dev/tty.usbmodem<xxxx>

#### Evaluating

When running the `heart_rate_monitor` example you should see:

    > Serial port used: /dev/tty.usbmodem<xxxx>
    > Warning: Successfully opened /dev/tty.usbmodem<xxxx>. Baud rate: 115200. Flow control: none. Parity: none.

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

When running the `heart_rate_collector` example, the nRF5 device is scanning as a BLE Master. You can experiment with this example more by flashing a separate nRF5 DK with the BLE Peripheral heart rate monitor example in the nRF5 SDK `nRF5_SDK_ROOT_PATH/examples/ble_peripheral/ble_app_hrs` so the two devices can interact.
