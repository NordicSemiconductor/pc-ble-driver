#!/usr/bin/python

# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import argparse
import distutils.archive_util
import os
import logging
import posixpath
import shutil

import sys
import tempfile
from sdk import Sdk

sys.dont_write_bytecode = True

import clean
from config import config
import examples
import serialization_dll
import tests
import utility
from patch import Patch

logger = logging.getLogger(__file__)


def package_release():
    package_name = config.get_serialization_release_name()
    package_path = posixpath.join(config.BUILD_DIR, package_name)

    utility.remove_directory(package_path)
    shutil.copytree(config.RELEASE_DIR, package_path)

    package_format = 'gztar'

    if config.PLATFORM_SYSTEM == 'Windows':
        package_format = 'zip'

    filename = distutils.archive_util.make_archive(package_path, package_format,
                                                   root_dir=config.BUILD_DIR, base_dir=package_name)

    logger.info("Release artifact package filename is \'%s\'", filename)


def parse_command_line():
    """Parses arguments and calls the appropriate functions"""
    parser = argparse.ArgumentParser(description='Builds the serialization dll')

    parser.add_argument('-b', '--build', action='store_true', help='Build the nrf51_ble_driver')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='Build folder should be cleaned before anything else happens')
    parser.add_argument('-d', '--dependencies', action='store_true', help='Retrieve dependencies')
    parser.add_argument('-e', '--examples', action='store_true', help='Build examples')
    parser.add_argument('-p', '--package', action='store_true', help='Package release folder')
    parser.add_argument('-t', '--test', action='store_true', help='Build and run tests')
    parser.add_argument('-v', '--version', metavar='VERSION', default='0.0.0',
                        help='Specify the version of the serialization dll being built')
    parser.add_argument('-r', '--revision', metavar='REVISION', default='0',
                        help='Specify the revision of the serialization dll being built')
    parser.add_argument('-n', '--version-name', metavar='VERSION_NAME', default='',
                        help='Specify the version name of the serialization dll being built')
    parser.add_argument('-sv', '--sdk-version', metavar='NRF51_SDK_VERSION', default='8.1.0',
                        help='Specify th SDK version to use when building th driver')
    parser.add_argument('-srp', '--sdk-root-path', metavar='NRF51_SDK_ROOT_PATH', required=True,
                        help='Specify the SDK root path where the build process stores the SDK')
    parser.add_argument('-cp', '--create-patch-filename', metavar='CREATE_PATCH', required=False,
                        help='Create a patch file in patch directory where from is downloaded SDK and is unpacked SDK path')

    return parser.parse_args()


def main():
    args = parse_command_line()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

    config.config_init(args.version, args.revision, args.version_name,
                       args.sdk_root_path, args.sdk_version,
                       root_dir)

    sdk = Sdk(config.ARTIFACTS_ROOT, config.SDK_VERSION)

    # Assumes that source root dir is .. since build.py is ran in directory build_scripts
    patch_dir = os.path.join(root_dir, 'patches')

    try:
        if args.clean:
            clean.clean_all(sdk)

        if args.create_patch_filename:
            sdk.prepare_sdk()
            old_path = sdk.path

            tempdir_path = None

            try:
                tempdir_path = tempfile.mkdtemp(prefix='nordicsemi')
                sdk.path = os.path.join(tempdir_path, "orig_sdk")
                sdk.prepare_sdk()
                p = Patch(patch_dir=patch_dir,
                          apply_patch_root_dir=sdk.path,
                          strip=0  # The old patches has 4 more components
                          )
                p.create_patch(tempdir_path, old_path, os.path.join(patch_dir, args.create_patch_filename))
            finally:
                sdk.path = old_path
                shutil.rmtree(tempdir_path)

        if args.dependencies:
            sdk.prepare_sdk()
            patch_tag_file = posixpath.join(sdk.path, ".patched")

            if os.path.exists(patch_tag_file):
                logger.info("Patches are already applied to this SDK, skipping patching")
            else:
                open(patch_tag_file, 'w').close()
                p = Patch(patch_dir=patch_dir,
                          apply_patch_root_dir=sdk.path,
                          strip=0  # The old patches has 4 more components
                          )
                p.apply_patches(dry_run=False)

        if args.build:
            serialization_dll.build()

        if args.test:
            error_code = tests.do_testing()

            if error_code != 0:
                return error_code

        if args.package:
            package_release()

        if args.examples:
            examples.build_examples()

    except Exception, ex:
        logger.exception(ex)
        return -1
    finally:
        for error_message in utility.get_message_log():
            logger.info(error_message)

    return 0

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    return_code = main()
    exit(return_code)
