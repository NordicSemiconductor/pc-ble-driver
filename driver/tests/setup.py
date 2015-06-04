#!/usr/bin/env python

# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
#
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import posixpath
import os
from setuptools import setup, find_packages
from setuptools.command.test import test as TestCommand
from setuptools_behave import behave_test

# To run the behave tests, remember to add argument --args bdd and then any other arguments you want to send to behave

def assert_required_environment_variable(envvar):
    if envvar not in os.environ:
        print('Environment variable {} not set. Exiting.'.format(envvar))
        exit(-1)
    else:
        return os.environ[envvar]

setup_py_path = os.path.dirname(os.path.realpath(__file__))

class NoseTestCommand(TestCommand):
    def finalize_options(self):
        TestCommand.finalize_options(self)
        self.test_args = []
        self.test_suite = True

    def run_tests(self):
        build_path = assert_required_environment_variable('NORDICSEMI_NRF51_BLE_DRIVER_BUILD_PATH')
        import nose

        zip_content_test_report_dir = posixpath.join(build_path, 'test_reports')
        zip_content_test_report_file = posixpath.join(zip_content_test_report_dir, 'zip_content.xml')

        if not os.path.exists(zip_content_test_report_dir):
            print("Directory {} does not exist. Creating it.".format(zip_content_test_report_dir))
            os.mkdir(zip_content_test_report_dir)

        nose.run_exit(argv=['nosetests',
                            '--where={0}'.format(os.path.join(setup_py_path, 'zip_content')),
                            '--with-xunit',
                            '--xunit-file={0}'.format(zip_content_test_report_file),
                            '--nocapture'])


nrfutil_version = "0.2.2"
nrfutil_git_link = "git+https://github.com/NordicSemiconductor/pc-nrfutil.git@v{0}#egg=nrfutil-{0}".format(nrfutil_version)


setup(name="tests",
      tests_require=[
          "nose >= 1.3.4",
          "behave == 1.2.5",
          "nrfutil == {0}".format(nrfutil_version),
      ],
      dependency_links=[
        nrfutil_git_link
      ],
      zip_safe=False,
      classifiers=[
          "Programming Language :: Python :: 2.7",
      ],
      cmdclass={
          'test': NoseTestCommand,
          'bdd_tests': behave_test
      }
      )

