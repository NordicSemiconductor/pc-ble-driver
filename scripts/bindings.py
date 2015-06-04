# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import os
import platform
import posixpath
import subprocess
import sys

sys.dont_write_bytecode = True

from config import config
import utility


def build_python_examples():
    examples_path = posixpath.join(config.RELEASE_DIR, 'python',
                                   'examples')
    examples_list = os.listdir(examples_path)

    pychecker_path = 'pychecker'

    if platform.system() == "Windows":
        pychecker_path = 'c:/python27/scripts/' + pychecker_path

    pychecker_command = pychecker_path + ' main.py'

    for example_name in examples_list:
        if not os.path.isdir(example_name):
            continue

        example_path = posixpath.join(examples_path, example_name)

        print 'Checking', example_name, 'example:'
        error_code = subprocess.call(pychecker_command, cwd=example_path, shell=True)

        if error_code != 0:
            utility.add_log_message('pychecker failed to verify {0} example.'.format(example_name))
