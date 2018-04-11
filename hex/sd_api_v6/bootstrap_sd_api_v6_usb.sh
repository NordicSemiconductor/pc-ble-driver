#!/bin/bash
#
# Download an patch the nRF5 SDK to compile the connectivity application.
# Use the nRF5 SDK 15.0 and the SoftDevice API version 6.
# Support usb.

ABS_PATH_USB="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH_USB/bootstrap_sd_api_v6.sh

echo "> Applying SDK patch for USB..."
patch \
  -p1 -s --ignore-whitespace \
  -d $ABS_PATH_USB/../../sdk/nRF5_SDK_15.0.0_a53641a \
  -i $ABS_PATH_USB/sdk150_connectivity_usb.patch
echo "> SDK for USB ready to use."
