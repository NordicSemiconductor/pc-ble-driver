# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import platform
import re
import os
from nrf_exceptions import *
import subprocess
import logging
import posixpath

logger = logging.getLogger('patch')


class Patch:
    def __init__(self, apply_patch_root_dir, patch_path='patch', patch_dir='patches', strip=0, diff_path='diff'):
        """
        Constructor, if patch_path does not point to an executable an nRFException is thrown.

        :param patch_path: The full path to the patch utility
        :param patch_dir: The directory containing the unified patches to apply
        :param strip: The number of leading components to strip from file names
        :return:
        """
        self.patch_path = patch_path
        self.patch_directory = patch_dir
        self.apply_patch_root_dir = apply_patch_root_dir
        self.patch_strip = strip
        self.diff_path = diff_path

        try:
            proc = subprocess.Popen(
                [self.patch_path, '--help'],
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE,
            )
            proc.wait()
            if proc.returncode != 0:
                raise nRFException("Error running patch command utility \'%s\'" % proc.stderr.read())
        except:
            raise nRFException('Not able to start patch command utility \'%s\'' % self.patch_path)

    def apply_patch(self, patch_file, dry_run=False):
        """
        Applies a patch file by running the patch command
        :param patch_file: The path to the patch file
        :return:
        """
        patch_args = [self.patch_path,
                      '-d', '%s' % self.apply_patch_root_dir,
                      '-t',  # ask not questions
                      '-u',  # unified patch file
                      '-N',  # Ignore patches that appear to be reversed or already applied.
                      '-p{}'.format(self.patch_strip),  # Strip n components from filename
                      '-i', '%s' % patch_file  # patch to apply
        ]

        if dry_run:
            patch_args.insert(0, '--dry-run')

        try:
            proc = subprocess.Popen(
                patch_args,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE,
                shell=False)

            proc.wait()

            if proc.returncode != 0:
                err_msg = 'Failed to apply patch \'%s\'\n. Error:%s\nError code from patch utility: %u' % (
                    patch_file,
                    proc.stdout.read(),
                    proc.returncode)
                logger.error(err_msg)
                raise nRFException(err_msg)

            logger.info(proc.stdout.read())
        finally:
            pass

    def apply_patches(self, dry_run=False):
        """
        Applies all patches found in directory specified in constructor. Patches must end with .patch
        :return:
        """
        if not os.path.isdir(self.patch_directory):
            raise nRFException("Path '%s' not found or is not a directory" % self.patch_directory)

        patch_files = sorted(os.listdir(self.patch_directory))

        for patch_file in patch_files:
            path = os.path.join(self.patch_directory, patch_file)
            if os.path.isfile(path) and path.endswith('.patch'):
                if platform.system() != "Windows":
                    # Convert files in SDK to Unix line endings \n so that the patch utility is
                    # able to patch the files with files from patch_directory.
                    files = self.get_files_to_patch(path)
                    for _file in files:
                        self._dos2unix(_file)

                self.apply_patch(path)
                logger.info("Successfully applied patch: %s" % path)

    def get_files_to_patch(self, patch_file, strip_components=0):
        files = []

        with open(patch_file, 'r') as patch:
            lines = patch.readlines()

            for line in lines:
                match = re.match("^---\s([\w/\d\.]+)[\t]?.*", line)

                if match:
                    relative_path = "/".join(match.group(1).split("/")[strip_components:])
                    abs_path = posixpath.join(self.apply_patch_root_dir, relative_path)
                    files.append(abs_path)

        return files

    @staticmethod
    def _dos2unix(filename):
        text = open(filename, 'rb').read().replace('\r\n', '\n')
        with open(filename, 'wb') as output:
            output.write(text)

    def create_patch(self, from_dir, to_dir, patch_file, binary=False):
        """
        Creates a patch file
        :param from_dir: Directory to compare against
        :param to_dir: Directory to compare against
        :param patch_file: Filename for patch
        :param binary: Create a binary diff, if not a text diff will be done
        :return:
        """

        diff_args = \
            [
                self.diff_path,
                '-r',  # recursively compare any subdirectories found
                '-u',  # unified patch file
                '-w',  # ignore whitespace
                '-P',  # treat absent first files as empty
                '-N',  # treat absent files as empty
                '--binary' if binary else '--text',
                from_dir,
                to_dir
            ]

        try:
            logger.debug("Starting diff: %s", " ".join(diff_args))
            proc = subprocess.Popen(
                diff_args,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE,
                shell=False)

            proc.wait()

            # GNU diffutils 3.0 exit/return codes used below
            if proc.returncode == 2:
                err_msg = 'Failed to generate patch file \'%s\'. Error:%s. Error code from diff utility: %u' % (
                    patch_file,
                    proc.stderr.read(),
                    proc.returncode)
                raise nRFException(err_msg)
            elif proc.returncode == 1:
                logger.info("Differences found, they are saved in the patch file %s", patch_file)
                patch_data = proc.stdout.read()
                with open(patch_file, 'wb') as _file:
                    _file.write(patch_data)
            elif proc.returncode == 0:
                logger.info("No differences found.")
        finally:
            pass

if __name__ == '__main__':
    logging.basicConfig()
    logger.setLevel(logging.DEBUG)
    logger.info("About to apply patches.")
    root_dir = os.path.realpath("..")
    p = Patch(apply_patch_root_dir=root_dir, patch_dir=os.path.join(root_dir, "patches"))
    p.apply_patches(dry_run=True)
