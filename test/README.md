# Running tests

Each of the tests in this directory require environment variables to run.

## Environment variables
The following environment variables affects the running of the tests:

| Environment variable                 | Description                                                                                        |
| -------------------------------------| ---------------------------------------------------------------------------------------------------|
| BLE_DRIVER_TEST_BAUD_RATE            | The baud rate to use in communication. Defaults to platform defaults if not provided.
| BLE_DRIVER_TEST_SERIAL_PORT_A        | The serial port to use in the tests.
| BLE_DRIVER_TEST_SERIAL_PORT_B        | The serial port to use in the tests. Used in tests where two devices are needed.
| BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS | The number of open close iterations to run before concluding the test. It defaults to 100 iterations. 