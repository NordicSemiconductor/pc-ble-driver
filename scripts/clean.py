# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

""""Module for cleaning up build folder"""

import posixpath

import utility

import sys
sys.dont_write_bytecode = True

from config import config

IGNORE_RM_TREE_ERRORS = True


def clean_all(sdk):
    """Removes _build folder and its content"""
    clean_dependencies(sdk)
    clean_build()


def clean_build():
    """Removes build folders and files"""

    c_objects_path = posixpath.join(config.BUILD_DIR)
    utility.remove_folder_content(c_objects_path)


def clean_dependencies(sdk):
    """Deletes SDK's that has been unpacked"""
    sdk.clean_sdk()
