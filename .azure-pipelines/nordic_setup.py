#!/usr/bin/env python

from urllib.parse import urlparse
import json
import sys
import os
import logging
import tempfile

import certifi
import pycurl


class DownloadCache:
    def __init__(self, cache_directory):
        self.cache_directory = cache_directory

    def __full_path(self, filename):
        return os.path.join(self.cache_directory, filename)

    def exists(self, filename, sha512):
        if os.path.exists(self.__full_path(filename)):
            if sha512 is None:
                return True
            else:
                # TODO: SHA checksum
                return False
        else:
            return False

    def path(self, filename):
        return self.__full_path(filename)

    def __str__(self):
        return f"[cache_directory:{self.cache_directory}]"


class Downloader:
    def __init__(self, download_cache):
        self.download_cache = download_cache

    def download(self, url, filename=None, sha512=None):
        # TODO: extract download filename
        download_url = url

        download_filename = None

        if filename is None:
            download_url = urlparse(url)
            download_filename = download_url.path.split("/")[-1]
        else:
            download_filename = filename

        logging.debug(
            'Checking if requested file %s to download exists in local cache and has valid sha512', download_filename)
        if self.download_cache and self.download_cache.exists(download_filename, sha512):
            return self.download_cache.path(download_filename)

        download_path = None

        if self.download_cache:
            download_path = self.download_cache.path(download_filename)
        else:
            download_path = os.path.join(
                tempfile.TemporaryDirectory(
                    prefix='nordic_toolchain'),
                download_filename
            )

        # Download SDK
        logging.debug(
            'Starting to download SDK from %s. Stored filename is %s', url, download_filename)
        with open(download_path, "wb") as download_file:
            curl_handle = pycurl.Curl()
            curl_handle.setopt(curl_handle.URL, download_url)
            curl_handle.setopt(curl_handle.WRITEDATA, download_file)
            curl_handle.setopt(curl_handle.FOLLOWLOCATION, True)
            curl_handle.setopt(curl_handle.MAXREDIRS, 5)
            curl_handle.setopt(curl_handle.CAINFO, certifi.where())
            curl_handle.perform()
            curl_handle.close()

        # Check sha512

    def __str__(self):
        return f"[download_cache:{self.download_cache}]"


class Compiler:
    def __init__(self, url, filename, sha512):
        self.url = url
        self.filename = filename
        self.sha512 = sha512

    def download(self, downloader):
        if self.url is None:
            raise Exception("No download info specified for compiler")

        downloader.download(self.url,
                            self.filename,
                            self.sha512)

    def install(self, prefix):
        pass

    def __str__(self):
        return f"[url:{self.url} filename:{self.filename} sha512:{self.sha512}]"


class Toolchain:
    def __init__(self, version, compiler, downloader):
        self.version = version
        self.compiler = compiler
        self.downloader = downloader

    def __str__(self):
        return f"[version:{self.version} compiler:{str(self.compiler)} downloader:{str(self.downloader)}]"

    def setup(self, install_prefix):
        if self.compiler is not None:
            self.compiler.download(self.downloader)
            self.compiler.install(install_prefix)


def get_toolchains():
    downloader = Downloader(DownloadCache("C:\\tmp\\dlcache"))
    toolchains_file = 'toolchains.json'
    toolchains = {}

    with open(toolchains_file, 'r') as file:
        j = json.load(file)

        if j.get('toolchains') is None:
            raise Exception(f'Unexpected format of {toolchains_file}')

        toolchains = j.get('toolchains')

        for toolchain_key in toolchains.keys():
            toolchain = toolchains.get(toolchain_key)

            if toolchain.get('platforms') is None:
                raise Exception(
                    f'Not platforms supported for toolchain {toolchain_key}')

            platforms = toolchain.get('platforms')

            if platforms is None:
                raise Exception(
                    'Unexpected format, expected platforms attribute')

            platform = platforms.get(sys.platform)

            if platform is None:
                raise Exception(
                    f'No toolchain found for platform {sys.platform}')

            compiler = platform.get('compiler')

            if compiler is None:
                logging.warning('No compiler found for platform %s', platform)

            toolchains[toolchain_key] = Toolchain(
                toolchain_key,
                Compiler(
                    compiler.get('url'),
                    compiler.get('filename'),
                    compiler.get('sha512')
                ),
                downloader
            )

        return toolchains


def setup_nordic_toolchain(toolchains_version, install_prefix):
    toolchain = get_toolchains()[toolchains_version]

    if toolchain is None:
        raise Exception(f"Toolchain version {toolchains_version} not found")

    toolchain.setup(install_prefix)


if __name__ == "__main__":
    logging.basicConfig()
    setup_nordic_toolchain("nRF5_SDK_v16", "c:\\tmp")
