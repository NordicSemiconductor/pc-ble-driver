# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import os
import posixpath
from nordicsemi.utility.target_registry import *


def before_all(context):
    filename = os.environ['NORDICSEMI_TARGET_SETUP']
    build_path = os.environ['NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH']

    os.environ['PATH'] += os.pathsep + posixpath.join(build_path, 'release', 'driver', 'lib')

    root_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..")
    context.resources_dir = os.path.join(root_dir, "resources")
    context.build_dir = os.path.join(build_path)
    context.release_dir = os.path.join(context.build_dir, "release")

    context.target_registry = TargetRegistry(target_db=FileTargetDatabase(filename))

    s110 = "connectivity_115k2_with_s110_7.1.0.hex"
    s120 = "connectivity_115k2_with_s120_1.0.1.hex"
    s130 = "connectivity_115k2_with_s130_1.0.0.hex"

    context.firmware = {
        'S110': os.path.join(context.resources_dir, s110),
        'S120': os.path.join(context.resources_dir, s120),
        'S130': os.path.join(context.resources_dir, s130),
        'blinky': os.path.join(root_dir, 'driver', 'tests', 'resources', 'blinky_nrf51422_xxac.hex')
    }

