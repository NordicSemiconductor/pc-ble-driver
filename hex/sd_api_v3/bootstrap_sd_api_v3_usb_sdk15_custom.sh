#!/bin/bash
#
# Download an patch the nRF5 SDK to compile the connectivity application.
# Use the nRF5 SDK 15.0 and the SoftDevice API version 6.

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH/../bootstrap.sh \
  -l 'insert/valid/path/to/nRF5_SDK_15.0.0_2e1b341.zip' \
  -f 'https://developer.nordicsemi.com/nRF5_SDK/pieces/nRF_DeviceFamilyPack/NordicSemiconductor.nRF_DeviceFamilyPack.8.16.0.pack' \
  -d "../sdk" \
  -p 'sd_api_v3/sdk150_connectivity_2e1b341.patch'
