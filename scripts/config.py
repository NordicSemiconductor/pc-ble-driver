# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

"""Module contains constants used in building the serialization dll"""
import logging

import platform
import posixpath
import os

import sys
from sdk import Sdk

logger = logging.getLogger(__file__)
sys.dont_write_bytecode = True


class config:
    def __init__(self):
        pass

    PRODUCT_NAME = "nRF51 Bluetooth Smart GATT/GAP Driver"

    DEPLOY_TARGET_NAME = 'nrf51-ble-driver'
    LIB_NAME = 'nrf51_ble_driver'

    # SDK related information
    SDK_VERSION = None
    ARTIFACTS_ROOT = None

    # Platform specific
    DYNAMIC_LIB_PREFIX = ''
    DYNAMIC_LIB_SUFFIX = ''
    IMPORT_LIB_SUFFIX = ''
    EXECUTABLE_PREFIX = ''
    EXECUTABLE_SUFFIX = ''
    PYTHON_BINDING_SUFFIX = ''

    PLATFORM_MAKE = ''
    PLATFORM_LIBS = []

    PLATFORM_TEST_BUILD_DIR = ''

    PLATFORM_SYSTEM = platform.system()

    REPO_ROOT_DIR = None
    BUILD_SCRIPT_DIR = None
    CODE_DIR = None
    RESOURCE_DIR = None
    BINDINGS_DIR = None
    PY_BINDINGS_DIR = None
    TEST_DIR = None

    BUILD_DIR = None
    SERIALIZATION_BUILD_DIR = None
    BINDINGS_BUILD_DIR = None
    PY_BINDINGS_BUILD_DIR = None
    TEST_REPORT_DIR = None
    RELEASE_DIR = None
    NRF51_SDK_DIR = None
    NRF51_SDK_SOURCE_DIR = None
    NRF51_SDK_INCLUDE_DIR = None

    if PLATFORM_SYSTEM == 'Windows':
        PLATFORM_SHORT_NAME = 'win'
        DYNAMIC_LIB_SUFFIX = '.dll'
        IMPORT_LIB_SUFFIX = '.lib'
        EXECUTABLE_SUFFIX = '.exe'
        PYTHON_BINDING_SUFFIX = '.pyd'

        PLATFORM_MAKE = 'mingw32-make'

        S110_TARGET_DEF_FILE = posixpath.join('_build', 'serialization_dll', 's110_nrf51_ble_driver.def')
        S110_TARGET_A_FILE = posixpath.join('_build', 'serialization_dll', 'libs110_nrf51_ble_driver.a')

        S120_TARGET_DEF_FILE = posixpath.join('_build', 'serialization_dll', 's120_nrf51_ble_driver.def')
        S120_TARGET_A_FILE = posixpath.join('_build', 'serialization_dll', 'libs120_nrf51_ble_driver.a')

        PLATFORM_TEST_BUILD_DIR = 'release'

    elif PLATFORM_SYSTEM == 'Darwin':
        PLATFORM_SHORT_NAME = 'osx'
        DYNAMIC_LIB_PREFIX = 'lib'
        DYNAMIC_LIB_SUFFIX = '.dylib'
        EXECUTABLE_PREFIX = './'
        PYTHON_BINDING_SUFFIX = '.so'

        PLATFORM_MAKE = 'make'
        PLATFORM_COMPILER = 'g++'
    elif PLATFORM_SYSTEM == 'Linux':
        PLATFORM_SHORT_NAME = 'linux'
        DYNAMIC_LIB_PREFIX = 'lib'
        DYNAMIC_LIB_SUFFIX = '.so'
        EXECUTABLE_PREFIX = './'
        PYTHON_BINDING_SUFFIX = '.so'

        PLATFORM_MAKE = 'make'
        PLATFORM_COMPILER = 'g++'
    else:
        print 'ERROR: Compiling on a unsupported platform.'
        exit(-1)

    VERSION = '0.0.0'
    REVISION = '0'
    VERSION_NAME = ''

    @staticmethod
    def set_version(version):
        config.VERSION = version

    @staticmethod
    def set_revision(revision):
        config.REVISION = revision

    @staticmethod
    def set_version_name(version_name):
        version_name = version_name.replace('\"', '')
        version_name = version_name.replace('\'', '')

        config.VERSION_NAME = version_name

    @staticmethod
    def get_serialization_release_name():
        release_name = config.DEPLOY_TARGET_NAME + '_' + config.PLATFORM_SHORT_NAME + '_' + config.VERSION

        if config.VERSION_NAME != '':
            release_name += '-' + config.VERSION_NAME

        return release_name

    @staticmethod
    def set_prefix_path(prefix_path, build_dir=os.getcwd()):
        config.REPO_ROOT_DIR = prefix_path
        config.BUILD_SCRIPT_DIR = posixpath.join(config.REPO_ROOT_DIR, 'scripts')
        config.CODE_DIR = posixpath.join(config.REPO_ROOT_DIR, 'driver')
        config.RESOURCE_DIR = posixpath.join(config.REPO_ROOT_DIR, 'resources')
        config.BINDINGS_DIR = posixpath.join(config.REPO_ROOT_DIR, 'python')
        config.PY_BINDINGS_DIR = config.BINDINGS_DIR
        config.TEST_DIR = posixpath.join(config.CODE_DIR, 'tests')

        config.BUILD_DIR = build_dir
        config.SERIALIZATION_BUILD_DIR = posixpath.join(config.BUILD_DIR, 'driver')
        config.BINDINGS_BUILD_DIR = posixpath.join(config.BUILD_DIR, 'python')
        config.PY_BINDINGS_BUILD_DIR = posixpath.join(config.BINDINGS_BUILD_DIR, 'python')
        config.TEST_REPORT_DIR = posixpath.join(config.BUILD_DIR, 'test_reports')
        config.RELEASE_DIR = posixpath.join(config.BUILD_DIR, 'release')
        config.NRF51_SDK_DIR = Sdk(config.ARTIFACTS_ROOT, config.SDK_VERSION).path
        config.NRF51_SDK_SOURCE_DIR = posixpath.join(config.NRF51_SDK_DIR, 'components')
        config.NRF51_SDK_INCLUDE_DIR = posixpath.join(config.NRF51_SDK_DIR, 'components')

    @staticmethod
    def set_nrf51_sdk_root_path(sdk_root_path):
        config.ARTIFACTS_ROOT = sdk_root_path

    @staticmethod
    def set_nrf51_sdk_version(sdk_version):
        config.SDK_VERSION = sdk_version

    @staticmethod
    def config_init(version, revision, version_name, sdk_root_path, sdk_version, prefix_path):
        config.set_version(version)
        config.set_revision(revision)
        config.set_version_name(version_name)
        config.set_nrf51_sdk_root_path(sdk_root_path)
        config.set_nrf51_sdk_version(sdk_version)
        config.set_prefix_path(prefix_path)
