#!/bin/bash
#
# Download SDK 12 to get SoftDevice s130 v2.0.1 which includes an important bugfix

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source $ABS_PATH/../bootstrap.sh --source-only
set_sdk_link 'https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/nRF5_SDK_12.1.0_0d23e2a.zip'
set_dl_location '../sdk'
sdk_download