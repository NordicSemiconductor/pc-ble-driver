# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

"""Updates version number in dll_details.rc"""
import sys


def rc_version_update(version='0.0.0', revision='0'):
    """Replaces current version and revision numbers in dll_details.rc"""

    rc_version_str = '"{0}.{1}\\0"'.format(version, revision)

    rc_version = version.replace('.', ',')
    rc_version += ',' + revision

    rc_file = open('dll_details.rc', 'r')
    rc_file_lines = rc_file.readlines()
    rc_file.close()

    for i in xrange(len(rc_file_lines)):
        line = rc_file_lines[i]
        is_define_line = "#define VER_" in line
        is_version_line = "VERSION" in line
        is_str_line = "_STR" in line

        if is_define_line and is_version_line:
            split_line_list = line.split(' ')

            if is_str_line:
                split_line_list[-1] = rc_version_str
            else:
                split_line_list[-1] = rc_version

            line = ' '.join(split_line_list)
            line += '\n'
            rc_file_lines[i] = line

    rc_file = open('dll_details.rc', 'w')
    rc_file.write(''.join(rc_file_lines))
    rc_file.close()


if __name__ == "__main__":
    version = '0.0.0'
    revision = '0'

    if len(sys.argv) > 1:
        version = sys.argv[1]

    if len(sys.argv) > 2:
        revision = sys.argv[2]

    rc_version_update(version, revision)
