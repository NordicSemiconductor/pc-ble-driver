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
| BLE_DRIVER_TEST_LOGLEVEL             | Specifies the pc-ble-driver log level. Defaults to 'info'. Can be 'trace', 'debug','info','warning','error','fatal'.

## Creating and running test targets
For CMake to create test targets one have to provide defines that identify the boards that are available on the system.

The format of the assigned values are <SEGGER_SERIAL_NUMBER>:<SERIAL_PORT>.

Since most integration tests require two boards the tests are setup to expect two boards of the same type in the tests. That way peripheral and central role for the same connectivity firmware can be ran in one test. If a board identifier value is not set, test targets for that combination of SoftDevice and board will not be created.

| Define (-D)                          | Description                                      |
|--------------------------------------|--------------------------------------------------|
| BLE_DRIVER_TEST_PCA10028_A           | Id for PCA10028 board A                          |
| BLE_DRIVER_TEST_PCA10028_B           | Id for PCA10028 board B                          |
| BLE_DRIVER_TEST_PCA10031_A           | Id for PCA10031 board A                          |
| BLE_DRIVER_TEST_PCA10031_B           | Id for PCA10031 board B                          |
| BLE_DRIVER_TEST_PCA10040_A           | Id for PCA10040 board A                          |
| BLE_DRIVER_TEST_PCA10040_B           | Id for PCA10040 board B                          |
| BLE_DRIVER_TEST_PCA10056_A           | Id for PCA10056 board A (using Segger USB CDC)   |
| BLE_DRIVER_TEST_PCA10056_B           | Id for PCA10056 board B (using Segger USB CDC)   |
| BLE_DRIVER_TEST_PCA10056_USB_A       | Id for PCA10056 board A (using USB CDC port)     |
| BLE_DRIVER_TEST_PCA10056_USB_B       | Id for PCA10056 board B (using USB CDC port)     |
| TEST_SOFTDEVICE_API                  | value set: test SoftDevice API, if not, skip     |
| TEST_TRANSPORT                       | value set: test transport layers, if not, skip   |
| TEST_ALL                             | value set: enable all tests                      |

For example, if you have two PCA10056 boards, you provide the following defines the CMake when generating the project files:
 -DBLE_DRIVER_TEST_PCA10056_A=<SEGGER_SERIAL_NUMBER>:<SERIAL_PORT> -DBLE_DRIVER_TEST_PCA10056_B=<SEGGER_SERIAL_NUMBER>:<SERIAL_PORT> -DTEST_SOFTDEVICE_API=1

The test targets needs to be ran manually, the naming of the test targets are "test_run_sdv<SoftDevice_API_version>\_<SoftDevice_type>\_pca<PCA version>".
Each of the targets first program the devices with the correct connectivity firmware and then starts the test. The test result is written in junit format in directory test-reports below the the test build directory.
