# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import logging
import os
import posixpath
import subprocess
import sys
import copy

sys.dont_write_bytecode = True

logger = logging.getLogger(__file__)


def do_testing():
    test_executable_dir = posixpath.join(os.getcwd(), "driver", "tests")

    env = copy.copy(os.environ)
    logger.info("PATH environment variable is: %s", env['PATH'])
    error_code = subprocess.call(["ctest",
                                  "--no-compress-output",
                                  "-T",
                                  "Test"],
                                 cwd=test_executable_dir,
                                 env=env,
                                 shell=False)

    if error_code != 0:
        logger.fatal('Run of tests failed')

    return 0
