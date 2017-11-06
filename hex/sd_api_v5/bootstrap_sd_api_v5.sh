#!/bin/bash
#
# Download an patch the nRF5 SDK to compile the connectivity application.
# Use the nRF5 SDK 12.1 and the SoftDevice API version 3.

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH/../bootstrap.sh \
  -l 'https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v14.x.x/nRF5_SDK_14.0.0_3bcc1f7.zip' \
  -f 'https://developer.nordicsemi.com/nRF5_SDK/pieces/nRF_DeviceFamilyPack/NordicSemiconductor.nRF_DeviceFamilyPack.8.14.1.pack' \
  -d "../sdk" \
  -p 'sd_api_v5/sdk140_connectivity.patch'
