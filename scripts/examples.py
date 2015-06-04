# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import posixpath
import subprocess

import sys
sys.dont_write_bytecode = True

import bindings
from config import config
import os
import utility


def build_c_examples():
    examples_path = posixpath.join(config.RELEASE_DIR, 'driver', 'examples')

    examples_list = os.listdir(examples_path)

    for example_name in examples_list:
        example_path = posixpath.join(examples_path, example_name)

        if not os.path.isdir(example_path):
            continue

        # GCC
        gcc_example_path = posixpath.join(example_path, 'gcc')
        error_code = subprocess.call(config.PLATFORM_MAKE, cwd=gcc_example_path, shell=True)

        if error_code != 0:
            log_message = 'Failed to compile {0} example with gcc.'.format(example_name)
            utility.add_log_message(log_message)

        # MSVC
        if config.PLATFORM_SYSTEM == 'Windows':
            msvc_example_path = posixpath.join(example_path, 'msvc')
            error_code = subprocess.call(['msbuild', '{0}.vcxproj'.format(example_name)],
                                         cwd=msvc_example_path, shell=True)

            if error_code != 0:
                log_message = 'Failed to compile {0} example with msvc.'.format(example_name)
                utility.add_log_message(log_message)


def build_examples():
    if not os.path.exists(config.RELEASE_DIR):
        utility.add_log_message('Need a build before we are able to test build examples.')
        return

    build_c_examples()

    #  Python
    bindings.build_python_examples()
