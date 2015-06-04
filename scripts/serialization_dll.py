# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import os
import copy
import posixpath
import shutil
import subprocess
import sys
import logging

from sdk import Sdk


sys.dont_write_bytecode = True

from config import config
import utility

logger = logging.getLogger(__name__)


def file_list_append(file_list, dir_path, file_name):
    file_path = posixpath.join(dir_path, file_name)
    file_list.append(file_path)


def build():
    logger.info('Building serialization dll artifacts with CMake')

    sdk_info = Sdk(config.ARTIFACTS_ROOT, config.SDK_VERSION)

    utility.make_directory(config.SERIALIZATION_BUILD_DIR)

    if config.PLATFORM_SYSTEM == 'Windows':
        generator = 'MinGW Makefiles'
    elif config.PLATFORM_SYSTEM in ['Linux', 'Darwin']:
        generator = 'Unix Makefiles'
    else:
        raise SystemError('Unknown platform. Not able to determine generator.')

    cmake_environment = None

    # Remove any git/bin path in environment variable PATH
    if config.PLATFORM_SYSTEM == 'Windows':
        environment_path = os.environ['PATH']
        environment_path_list = environment_path.split(';')
        environment_path_list = [path for path in environment_path_list if 'Git\\bin' not in path]
        environment_path = ';'.join(environment_path_list)

        cmake_environment = copy.copy(os.environ)
        cmake_environment['PATH'] = environment_path

    for artifact in ['driver', 'binding']:
        cmake_args = ['cmake', '-G', '{0}'.format(generator),
                      '-DNRF51_SDK_PATH={0}'.format(sdk_info.path),
                      '-DSERIALIZATION_VERSION={0}'.format(config.VERSION),
                      '-DSERIALIZATION_REVISION={0}'.format(config.REVISION),
                      '-DARTIFACT={0}'.format(artifact),
                      config.REPO_ROOT_DIR]

        logging.debug("Starting to build with command: %s", " ".join(cmake_args))

        return_code = None

        try:
            return_code = subprocess.call(cmake_args,
                                          shell=False,
                                          env=cmake_environment)

            if return_code != 0:
                err_msg = 'Failed to prepare build of {0} libraries. Error code: {1}.'.format(artifact, return_code)
                utility.add_log_message(err_msg)
                raise SystemError(err_msg)

            return_code = subprocess.call([config.PLATFORM_MAKE], shell=True)

            if return_code != 0:
                err_msg = 'Failed to build artifact {0}. Error code: {0}.'.format(artifact, return_code)
                utility.add_log_message(err_msg)
                raise SystemError(err_msg)
        except Exception, e:
            logger.fatal(e)
            return return_code

    # Scrape together the built artifacts into release directory
    copy_serialization_dll_release_files()
    copy_python_binding_release_files()

    if config.PLATFORM_SYSTEM == 'Windows':
        copy_windows_lib()


def copy_serialization_dll(softdevice_name='s130'):
    dynamic_library_name = config.DYNAMIC_LIB_PREFIX + softdevice_name + '_' + config.LIB_NAME
    dynamic_library_file = dynamic_library_name + config.DYNAMIC_LIB_SUFFIX
    dynamic_library_path = posixpath.join(config.SERIALIZATION_BUILD_DIR,
                                          dynamic_library_file)
    dynamic_library_dest_path = posixpath.join(config.RELEASE_DIR, "driver", "lib")
    utility.make_directory(dynamic_library_dest_path)
    shutil.copy2(dynamic_library_path, dynamic_library_dest_path)


def copy_python_binding_release_files(softdevice_name='s130'):
    s130_bindings_release_path = posixpath.join(config.RELEASE_DIR, 'python')
    utility.make_clean_directory(s130_bindings_release_path)

    python_binding_lib_name = softdevice_name + '_' + config.LIB_NAME
    python_binding_py_name = python_binding_lib_name + '.py'
    python_binding_pyd_name = '_' + python_binding_lib_name + config.PYTHON_BINDING_SUFFIX

    python_bindings_build_path = posixpath.join(config.BUILD_DIR, 'python')
    python_binding_release_path = posixpath.join(config.RELEASE_DIR, 'python')

    utility.make_directory(python_binding_release_path)

    python_binding_py_path = posixpath.join(python_bindings_build_path,
                                            python_binding_py_name)
    python_binding_pyd_path = posixpath.join(python_bindings_build_path,
                                             python_binding_pyd_name)

    shutil.copy2(python_binding_py_path, python_binding_release_path)
    shutil.copy2(python_binding_pyd_path, python_binding_release_path)

    python_binding_path = config.BINDINGS_DIR
    python_binding_examples_path = posixpath.join(python_binding_path, 'examples')
    python_binding_examples_release_path = posixpath.join(python_binding_release_path, 'examples')

    python_bindings_util_path = posixpath.join(python_binding_path, 'src', 'ble_driver_util.py')
    shutil.copy2(python_bindings_util_path, python_binding_release_path)
    shutil.copytree(python_binding_examples_path, python_binding_examples_release_path)


def copy_windows_lib(softdevice_name='s130'):
    import_library_name = softdevice_name + '_' + config.LIB_NAME + config.IMPORT_LIB_SUFFIX
    import_library_path = posixpath.join(config.BUILD_DIR, 'driver',
                                         import_library_name)

    release_import_library_path = posixpath.join(config.RELEASE_DIR, 'driver', 'lib')

    utility.make_directory(release_import_library_path)

    shutil.copy2(import_library_path, release_import_library_path)


def copy_licenses():
    license_paths = [posixpath.join(config.REPO_ROOT_DIR, 'license.txt'),
                     posixpath.join(config.REPO_ROOT_DIR,
                                    'S130_license_agreement.pdf')]

    licenses_dest_path = config.RELEASE_DIR

    for license_path in license_paths:
        shutil.copy2(license_path, licenses_dest_path)


def copy_header_files(softdevice_name='s130'):
    serialization_header_path = posixpath.join(config.CODE_DIR, 'inc')
    include_override_path = posixpath.join(config.CODE_DIR, 'inc_override')
    softdevice_header_path = posixpath.join(config.NRF51_SDK_INCLUDE_DIR, 'softdevice',
                                            softdevice_name, 'headers')

    header_file_list = []

    file_list_append(header_file_list, serialization_header_path, 'sd_rpc.h')
    file_list_append(header_file_list, include_override_path, 'nrf_svc.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_hci.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_err.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_types.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_ranges.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_l2cap.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_gap.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_gatt.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_gattc.h')
    file_list_append(header_file_list, softdevice_header_path, 'ble_gatts.h')
    file_list_append(header_file_list, softdevice_header_path, 'nrf_error.h')

    release_include_path = posixpath.join(config.RELEASE_DIR, 'driver', 'include')

    utility.make_directory(release_include_path)

    for header_file_path in header_file_list:
        shutil.copy2(header_file_path, release_include_path)


def copy_documentation():
    documentation_path = posixpath.join(config.RESOURCE_DIR, 'README.md')
    release_documentation_path = config.RELEASE_DIR

    utility.make_directory(release_documentation_path)

    shutil.copy2(documentation_path, release_documentation_path)


def copy_hex_files(softdevice_name='s130'):
    hex_file_list = []

    version = {
        's110': '7.1.0',
        's120': '1.0.1',
        's130': '1.0.0'}[softdevice_name]

    hex_resource_path = config.RESOURCE_DIR

    hex_file_name = 'connectivity_115k2_with_{0}_{1}.hex'.format(softdevice_name, version)
    file_list_append(hex_file_list, hex_resource_path, hex_file_name)

    if not config.PLATFORM_SYSTEM == 'Darwin':
        hex_file_name = 'connectivity_1m_with_{0}_{1}.hex'.format(softdevice_name, version)
        file_list_append(hex_file_list, hex_resource_path, hex_file_name)

    release_hex_path = posixpath.join(config.RELEASE_DIR, 'firmware')

    utility.make_directory(release_hex_path)

    for hex_file_path in hex_file_list:
        shutil.copy2(hex_file_path, release_hex_path)


def ignored_example_files(src, files):
    ignore_dirs = ['qt']
    not_ignored_file_endings = ('.c', '.h', '.vcxproj', 'Makefile', 'Makefile.common')
    ignored_files = []

    if config.PLATFORM_SYSTEM != 'Windows':
        ignore_dirs.append('msvc')

    for file_name in files:
        file_path = posixpath.join(src, file_name)

        if os.path.isdir(file_path):

            if file_name in ignore_dirs:
                ignored_files.append(file_name)

        elif not file_name.endswith(not_ignored_file_endings):
            ignored_files.append(file_name)

    return ignored_files


def copy_example_files():
    examples_dir = posixpath.join(config.CODE_DIR, 'examples')
    examples_destination = posixpath.join(config.RELEASE_DIR, 'driver', 'examples')

    utility.remove_directory(examples_destination)

    shutil.copytree(examples_dir, examples_destination, ignore=ignored_example_files)

    stdbool_h_path = posixpath.join(examples_dir, 'stdbool.h')
    shutil.copy2(stdbool_h_path, examples_destination)


def copy_serialization_dll_release_files():
    logger.info('Copying built artifacts to %s.', config.RELEASE_DIR)

    utility.make_clean_directory(config.RELEASE_DIR)

    copy_licenses()
    copy_documentation()
    copy_serialization_dll()

    if config.PLATFORM_SYSTEM == 'Windows':
        copy_windows_lib()

    copy_header_files()
    copy_hex_files()
    copy_example_files()
