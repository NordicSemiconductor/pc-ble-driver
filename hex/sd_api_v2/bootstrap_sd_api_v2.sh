#!/bin/bash
#
# Download an patch the nRF5 SDK to compile the connectivity application.
# Use the nRF5 SDK 11 and the SoftDevice API version 2.

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH/../bootstrap.sh \
  -l 'https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip' \
  -f 'https://developer.nordicsemi.com/nRF5_SDK/pieces/nRF_DeviceFamilyPack/NordicSemiconductor.nRF_DeviceFamilyPack.8.5.0.pack' \
  -d "../sdk" \
  -p 'sd_api_v2/sdk110_connectivity.patch'
