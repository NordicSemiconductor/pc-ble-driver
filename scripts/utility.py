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
import shutil
import hashlib
import posixpath

import sys
sys.dont_write_bytecode = True

message_log = []

logger = logging.getLogger(__name__)


def remove_directory(directory_path):
    tries = 0

    while os.path.exists(directory_path):
        try:
            shutil.rmtree(directory_path, ignore_errors=True)
        except:
            logger.fatal("FAILED TO REMOVE DIRECTORY %s", directory_path)

        tries += 1
        if tries > 9:
            break

def remove_folder_content(directory_path):
    for root, dirs, files in os.walk(directory_path, topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))


def make_directory(directory_path):
    tries = 0

    while not os.path.exists(directory_path):
        try:
            os.makedirs(directory_path)
        except:
            pass

        tries += 1
        if tries > 9:
            logger.fatal("FAILED TO CREATE DIRECTORY %s", directory_path)


def make_clean_directory(directory_path):
    remove_directory(directory_path)

    make_directory(directory_path)


def add_log_message(message):
    global message_log
    message_log.append(message)


def get_message_log():
    global message_log
    return message_log


def md5sum(filename, block_size=65536):
    _hash = hashlib.md5()
    with open(filename, "r+b") as f:
        for block in iter(lambda: f.read(block_size), ""):
            _hash.update(block)
    return _hash.hexdigest().upper()
