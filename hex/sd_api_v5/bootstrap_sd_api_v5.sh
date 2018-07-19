#!/bin/bash
#
# Download and patch the nRF5 SDK to compile the connectivity application.
# Use the nRF5 SDK 15 and the SoftDevice API version 5.

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH/../bootstrap.sh \
  -l 'https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.0.0_a53641a.zip' \
  -f 'https://developer.nordicsemi.com/nRF5_SDK/pieces/nRF_DeviceFamilyPack/NordicSemiconductor.nRF_DeviceFamilyPack.8.16.0.pack' \
  -d "../sdk" \
  -p 'sdk150_add_sd_v3v5_support.patch'
