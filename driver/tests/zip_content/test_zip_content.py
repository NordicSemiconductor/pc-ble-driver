# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
#
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import fnmatch
import os
import platform
import posixpath
import unittest
import tarfile
from zipfile import ZipFile

class TestZipContent(unittest.TestCase):

    def test_zip_content(self):
        os_str = ''
        build_dir = os.environ['NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH']
        dynamic_library_prefix = ''
        dynamic_library_suffix = ''
        python_bindings_suffix = ''

        if platform.system() == 'Windows':
            os_str = 'win'
            dynamic_library_suffix = '.dll'
            python_bindings_suffix = '.pyd'

        if platform.system() == 'Linux':
            os_str = 'linux'
            dynamic_library_prefix = 'lib'
            dynamic_library_suffix = '.so'
            python_bindings_suffix = '.so'

        if platform.system() == 'Darwin':
            os_str = 'darwin'
            dynamic_library_prefix = 'lib'
            dynamic_library_suffix = '.dylib'
            python_bindings_suffix = '.so'

        archive_name = 'nrf51-ble-driver'

        archive_filter = archive_name + '*'

        if platform.system() == 'Windows':
            archive_filter = archive_filter + '.zip'
        else:
            archive_filter = archive_filter + '.tar.gz'

        files_in_archive_dir = os.listdir(build_dir)

        archive_matches = fnmatch.filter(files_in_archive_dir, archive_filter)

        self.assertTrue(len(archive_matches) > 0)
        archive_name = archive_matches[0]
        archive_path = posixpath.join(build_dir, archive_name)

        archive_base_name = archive_name
        archive_base_name = archive_base_name.replace(".zip", "")
        archive_base_name = archive_base_name.replace(".tar.gz", "")

        expected_file_list = ['README.md',
                              'license.txt',
                              'S130_license_agreement.pdf',

                              'driver/examples/Makefile.common',
                              'driver/examples/stdbool.h',
                              'driver/examples/advertising/main.c',
                              'driver/examples/advertising/gcc/Makefile',
                              'driver/examples/heart_rate_collector/main.c',
                              'driver/examples/heart_rate_collector/gcc/Makefile',
                              'driver/examples/heart_rate_monitor/main.c',
                              'driver/examples/heart_rate_monitor/gcc/Makefile',
                              'driver/examples/heart_rate_relay/main.c',
                              'driver/examples/heart_rate_relay/gcc/Makefile',
                              'driver/examples/multi_link/main.c',
                              'driver/examples/multi_link/gcc/Makefile',

                              'driver/include/ble.h',
                              'driver/include/ble_err.h',
                              'driver/include/ble_gap.h',
                              'driver/include/ble_gatt.h',
                              'driver/include/ble_gattc.h',
                              'driver/include/ble_gatts.h',
                              'driver/include/ble_hci.h',
                              'driver/include/ble_l2cap.h',
                              'driver/include/ble_ranges.h',
                              'driver/include/ble_types.h',
                              'driver/include/nrf_error.h',
                              'driver/include/nrf_svc.h',
                              'driver/include/sd_rpc.h',

                              'driver/lib/{0}s130_nrf51_ble_driver{1}'.format(dynamic_library_prefix, dynamic_library_suffix),

                              'firmware/connectivity_115k2_with_s130_1.0.0.hex',

                              'python/ble_driver_util.py',
                              'python/s130_nrf51_ble_driver.py',
                              'python/_s130_nrf51_ble_driver{0}'.format(python_bindings_suffix),
                              'python/examples/advertising/main.py',
                              'python/examples/heart_rate_monitor/main.py',
                              'python/examples/heart_rate_collector/main.py',
                              'python/examples/heart_rate_relay/main.py',
                              'python/examples/multi_link/main.py']

        if platform.system() != 'Darwin':
          expected_file_list.extend(['firmware/connectivity_1m_with_s130_1.0.0.hex'])

        if platform.system() == 'Windows':
            expected_file_list.extend(['driver/examples/advertising/msvc/advertising.vcxproj',
                                       'driver/examples/heart_rate_collector/msvc/heart_rate_collector.vcxproj',
                                       'driver/examples/heart_rate_monitor/msvc/heart_rate_monitor.vcxproj',
                                       'driver/examples/heart_rate_relay/msvc/heart_rate_relay.vcxproj',
                                       'driver/examples/multi_link/msvc/multi_link.vcxproj',
                                       'driver/lib/s130_nrf51_ble_driver.lib'])
        else:
            expected_file_list.extend(['driver',
                                       'driver/examples',
                                       'driver/examples/advertising',
                                       'driver/examples/advertising/gcc',
                                       'driver/examples/heart_rate_collector',
                                       'driver/examples/heart_rate_collector/gcc',
                                       'driver/examples/heart_rate_monitor',
                                       'driver/examples/heart_rate_monitor/gcc',
                                       'driver/examples/heart_rate_relay',
                                       'driver/examples/heart_rate_relay/gcc',
                                       'driver/examples/multi_link',
                                       'driver/examples/multi_link/gcc',
                                       'driver/include',
                                       'driver/lib',
                                       'firmware',
                                       'python',
                                       'python/examples',
                                       'python/examples/advertising',
                                       'python/examples/heart_rate_collector',
                                       'python/examples/heart_rate_monitor',
                                       'python/examples/heart_rate_relay',
                                       'python/examples/multi_link'])

        expected_file_list = map(lambda x: archive_base_name + '/' + x, expected_file_list)

        if not platform.system() == 'Windows':
            expected_file_list.append(archive_base_name)

        actual_file_list = []

        if platform.system() == 'Windows':
            archive = ZipFile(archive_path)
            actual_file_list.extend(archive.namelist())
        else:
            archive = tarfile.open(archive_path)
            actual_file_list.extend(archive.getnames())

        expected_file_list = sorted(expected_file_list)
        actual_file_list = sorted(actual_file_list)

        self.assertEqual(actual_file_list, expected_file_list)


if __name__ == '__main__':
    unittest.main()
