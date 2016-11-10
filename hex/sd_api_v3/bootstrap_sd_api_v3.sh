#!/bin/bash

source ../bootstrap.sh

#SDK_LINK='https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip'
set_sdk_link 'http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/nRF5_SDK_12.1.0_0d23e2a.zip'

set_dl_location ../tmp

set_patch_file 'sdk121_connectivity.patch'

run
