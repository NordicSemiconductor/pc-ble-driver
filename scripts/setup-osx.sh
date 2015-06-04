#!/bin/sh
#
# MacOSX installation
#

# Install XCode
git --version # This triggers install of xcode if not already installed. If another version of git is installed we assume that the user has install XCode already.

ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install cmake
brew install homebrew/versions/swig2
CFLAGS=-fPIC brew install --verbose  --env=std --build-from-source boost

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
