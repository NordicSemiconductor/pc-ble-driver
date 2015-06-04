#!/bin/sh
#
# Ubuntu Linux installation
#

# Install packages
sudo apt-get -y install cmake swig git libboost-all-dev g++

# Setup environment variables
export NORDICSEMI_NRF51_BLE_DRIVER_ROOT_PATH=$HOME/nordicsemi/driver
export NORDICSEMI_NRF51_BLE_DRIVER_DEPS_PATH=$NORDICSEMI_NRF51_BLE_DRIVER_ROOT_PATH/deps
export NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH=$NORDICSEMI_NRF51_BLE_DRIVER_ROOT_PATH/build
export NORDICSEMI_NRF51_BLE_DRIVER_SRC_PATH=$NORDICSEMI_NRF51_BLE_DRIVER_ROOT_PATH/pc-ble-driver
export NORDICSEMI_NRF51_BLE_DRIVER_GIT_URL=https://github.com/NordicSemiconductor/pc-ble-driver.git

# Create directories
mkdir -p $NORDICSEMI_NRF51_BLE_DRIVER_ROOT_PATH

# Compile and package driver (.tar.gz file) (no tests are run)
git clone $NORDICSEMI_NRF51_BLE_DRIVER_GIT_URL $NORDICSEMI_NRF51_BLE_DRIVER_SRC_PATH

# Let the user follow the instructions in the document instead of doing the steps below automatically.
# mkdir -p $NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH $NORDICSEMI_NRF51_BLE_DRIVER_DEPS_PATH
# cd $NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH
# python ../pc-ble-driver/scripts/build.py -srp $NORDICSEMI_NRF51_BLE_DRIVER_DEPS_PATH -d -b -p -e
