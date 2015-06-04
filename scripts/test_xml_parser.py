# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import sys


def write_test_fragment(file_path, xml_lines, from_line, to_line):

    if not xml_lines[from_line].startswith("<?xml"):
        return

    xml_fragment_lines = xml_lines[from_line:to_line]
    xml_fragment = ''.join(xml_fragment_lines)

    xml_fragment_file_name = file_path + '.converted_'
    xml_fragment_file_name += str(from_line) + '-' + str(to_line) + '.xml'

    xml_fragment_file = open(xml_fragment_file_name, 'w')
    xml_fragment_file.write(xml_fragment)
    xml_fragment_file.close()


def split_test_xml(xml_file_path_list):
    """ Splits files on '<?' tag creating multiple 'converted' files.
        Makes it possible for Bamboo to parse the test output.
    """
    for xml_file_path in xml_file_path_list:
        xml_file = open(xml_file_path, 'r')
        xml_file_lines = xml_file.readlines()
        xml_file.close()

        if len(xml_file_lines) == 0:
            return

        i = 0
        last_xml_tag_index = 0
        xml_tag_index = 0
        while i < len(xml_file_lines):
            line = xml_file_lines[i]
            has_xml_tag = '<?' in line

            if has_xml_tag:
                xml_tag_index = i

                if last_xml_tag_index != xml_tag_index:
                    write_test_fragment(xml_file_path, xml_file_lines, last_xml_tag_index, xml_tag_index)

                last_xml_tag_index = xml_tag_index

            i += 1

        write_test_fragment(xml_file_path, xml_file_lines, last_xml_tag_index, i)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'Not enough input'
        exit(0)

    xml_file_path_list = sys.argv[1].split(';')

    split_test_xml(xml_file_path_list)


#TODO: Create test parser function
