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
import urllib
from zipfile import ZipFile
import utility

logger = logging.getLogger(__file__)

NRF51_SDK_DOWNLOAD_BASE_URL = "http://developer.nordicsemi.com/nRF51_SDK"


class Sdk:
    # Since the web pages do not have any API to retrieve directory structure
    # we have to hard code the version number towards download URL.

    # Since the web pages do not provide MD5 checksums for the files
    # we have to maintain it here
    NRF51_SDK_VERSION_URL_MAP = {
        "7.0.0": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v7.x.x/nRF51_SDK_7.0.0_2ab6a52.zip",
            "md5sum": "DFC5D3CDE20BA92504557B2C4CD94C00"
        },
        "7.0.1": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v7.x.x/nRF51_SDK_7.0.1_1d6e5cb.zip",
            "md5sum": "830A4E8EF50860B30D9F6824D083F36B"
        },
        "7.1.0": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v7.x.x/nRF51_SDK_7.1.0_372d17a.zip",
            "md5sum": "5B7B312B3AF0BF6CC7AD716DA47E955C"
        },
        "7.2.0": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v7.x.x/nRF51_SDK_7.2.0_cf547b5.zip",
            "md5sum": "84C9514E565A075D9773FAF0C2B5AC2B"
        },
        "8.0.0": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v8.x.x/nRF51_SDK_8.0.0_5fc2c3a.zip",
            "md5sum": "EAAAB0920422EB0FF2527609AE3CCD17"
        },
        "8.1.0": {
            "url": NRF51_SDK_DOWNLOAD_BASE_URL + "/nRF51_SDK_v8.x.x/nRF51_SDK_8.1.0_b6ed55f.zip",
            "md5sum": "BA544336B92A773B20CDE51E53EB4419"
        }
    }

    def __init__(self, artifacts_path, sdk_version):
        if not Sdk.version_exists(sdk_version):
            raise Exception("SDK version {} does not exist".format(sdk_version))

        if artifacts_path is None:
            raise Exception("artifacts_root must be specified")

        if sdk_version is None:
            raise Exception("sdk_version must be specified")

        # Normalize the artifacts path to posix since cmake nags if \ i used
        self.artifacts_path = artifacts_path.replace("\\", "/")
        self.version = sdk_version

        self.url = Sdk.NRF51_SDK_VERSION_URL_MAP[sdk_version]['url']
        self.zip_path = posixpath.join(self.artifacts_path, self.url.split("/")[-1])
        self.path = self.zip_path.replace(".zip", "")
        self.md5sum = Sdk.NRF51_SDK_VERSION_URL_MAP[sdk_version]['md5sum']

    def prepare_sdk(self):
        """
          Prepares the SDK by downloading the .zip file if necessary, and unpacks it
        """

        if not os.path.exists(self.artifacts_path):
            logger.info("Directory {} does not exist, creating it.".format(self.artifacts_path))
            os.mkdir(self.artifacts_path)

        if os.path.exists(self.zip_path):
            logger.info("SDK already downloaded, skipping download.")
        else:
            logger.info("Downloading SDK from %s", self.url)
            urllib.urlretrieve(self.url, self.zip_path)

        _hash = utility.md5sum(self.zip_path)
        if _hash != self.md5sum:
            raise Exception("Hash of downloaded SDK file is not correct. Is {}, should be {}".format(
                hash,
                self.md5sum)
            )

        if os.path.exists(self.path):
            logger.info("Assuming SDK is already unpacked since directory exists")
        else:
            logger.info("Unpacking SDK %s to %s", os.path.basename(self.zip_path), self.path)
            os.mkdir(self.path)

            with ZipFile(self.zip_path, 'r') as sdk_zip:
                sdk_zip.extractall(self.path)

    def clean_sdk(self):
        """Deletes SDK that has been unpacked"""
        utility.remove_directory(self.path)

    @staticmethod
    def version_exists(sdk_version):
        return sdk_version in Sdk.NRF51_SDK_VERSION_URL_MAP
