#!/usr/bin/env python
# Python 3.2 or later required

from urllib.parse import urlparse
import json
import sys
import os
import logging
import tempfile
import hashlib
import zipfile
import tarfile
import argparse
import pathlib
import tempfile
import re

import certifi
import pycurl


class NordicSetupException(Exception):
    def __init__(self, message):
        self.message = message


class DownloadCache:
    def __init__(self, cache_directory):
        self.cache_directory = cache_directory

        if not os.path.exists(cache_directory):
            pathlib.Path(self.cache_directory).mkdir(parents=True, exist_ok=True)

    def __full_path(self, filename):
        return os.path.join(self.cache_directory, filename)

    def exists(self, filename, sha512):
        if os.path.exists(self.__full_path(filename)):
            if sha512 is None:
                return True
            else:
                hash = hashlib.sha512()

                with open(self.__full_path(filename), 'rb') as file:
                    for block in iter(lambda: file.read(4096), b""):
                        hash.update(block)

                digest = hash.hexdigest()

                if digest == sha512:
                    return True

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
        download_url = url
        download_filename = None

        if filename is None:
            download_filename = urlparse(url).path.split("/")[-1]
        else:
            download_filename = filename

        logging.debug(
            'Checking if file %s to exists in '
            'local cache and has valid digest',
            download_filename
        )

        if self.download_cache and self.download_cache.exists(download_filename, sha512):
            logging.debug(
                'Cache hit for %s with sha512 %s',
                download_filename, sha512
            )
            return self.download_cache.path(download_filename)
        else:
            logging.debug(
                'Cache miss for %s with sha512 %s',
                download_filename, sha512)

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
            'Starting to download SDK from %s. Stored filename is %s',
            url,
            download_path
        )

        with open(download_path, "wb") as download_file:
            curl_handle = pycurl.Curl()
            curl_handle.setopt(curl_handle.URL, download_url)
            curl_handle.setopt(curl_handle.WRITEDATA, download_file)
            curl_handle.setopt(curl_handle.FOLLOWLOCATION, True)
            curl_handle.setopt(curl_handle.MAXREDIRS, 5)
            curl_handle.setopt(curl_handle.CAINFO, certifi.where())
            curl_handle.perform()
            curl_handle.close()

        # Check digest
        if self.download_cache:
            if not self.download_cache.exists(download_filename, sha512):
                raise NordicSetupException(f'{download_filename} file does not match digest {sha512}')

        return download_path

    def __str__(self):
        return f"[download_cache:{self.download_cache}]"


class GnuCompiler:
    def __init__(self, url, filename, sha512):
        self.url = url
        self.sha512 = sha512
        self.download_path = None

        if filename is None:
            self.filename = urlparse(url).path.split("/")[-1]
        else:
            self.filename = filename

    def download(self, downloader):
        if self.url is None:
            raise NordicSetupException(
                'No download info specified for compiler'
            )

        self.download_path = downloader.download(
            self.url,
            self.filename,
            self.sha512
        )

    def install(self, prefix):
        compiler_path = os.path.join(prefix, 'compiler')

        if not os.path.exists(compiler_path):
            pathlib.Path(compiler_path).mkdir(parents=True, exist_ok=True)

        logging.debug(
            'Installing %s into %s',
            self.download_path,
            compiler_path
        )

        if self.download_path.endswith('.zip'):
            with zipfile.ZipFile(self.download_path, 'r') as zip_ref:
                zip_ref.extractall(compiler_path)
        elif self.download_path.endswith('.tar.bz2'):
            with tarfile.open(self.download_path, 'r:bz2') as tar:
                tar.extractall(compiler_path)
            # tar packaged version of compiler contains a root directory
            compiler_path = os.path.join(compiler_path, re.search('\/([^/.]*)-[linux|mac]*\.tar\.bz2$', self.download_path).group(1))
        
        return [
            # Old env variable used by some projects
            f'GNUARMEMB_TOOLCHAIN_PATH={compiler_path}',
            # Newer env variable for compiler
            f'GCCARMEMB_TOOLCHAIN_PATH={compiler_path}'
        ]

    def __str__(self):
        return f"[url:{self.url} filename:{self.filename} "\
            "sha512:{self.sha512}]"


class Toolchain:
    def __init__(self, version, compiler, downloader):
        self.version = version
        self.compiler = compiler
        self.downloader = downloader

    def __str__(self):
        return f"[version:{self.version} compiler:{str(self.compiler)}"\
            " downloader:{str(self.downloader)}]"

    def setup(self, install_prefix):
        compiler_path = None
        env = []

        if self.compiler is not None:
            self.compiler.download(self.downloader)
            env.extend(self.compiler.install(install_prefix))

        return env


def get_toolchains(cache_directory):
    downloader = Downloader(DownloadCache(cache_directory))

    toolchains_file = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        'toolchains.json'
    )

    toolchains = {}

    with open(toolchains_file, 'r') as file:
        j = json.load(file)

        if j.get('toolchains') is None:
            raise NordicSetupException(
                f'Unexpected format of {toolchains_file}'
            )

        toolchains = j.get('toolchains')

        for toolchain_key in toolchains.keys():
            toolchain = toolchains.get(toolchain_key)

            if toolchain.get('platforms') is None:
                raise NordicSetupException(
                    f'Not platforms supported for toolchain {toolchain_key}')

            platforms = toolchain.get('platforms')

            if platforms is None:
                raise NordicSetupException(
                    'Unexpected format, expected platforms attribute')

            platform = platforms.get(sys.platform)

            if platform is None:
                raise NordicSetupException(
                    f'No toolchain found for platform {sys.platform}')

            compiler = platform.get('compiler')

            if compiler is None:
                logging.warning('No compiler found for platform %s', platform)

            toolchains[toolchain_key] = Toolchain(
                toolchain_key,
                GnuCompiler(
                    compiler.get('url'),
                    compiler.get('filename'),
                    compiler.get('sha512')
                ),
                downloader
            )

        return toolchains


def setup_nordic_toolchain(toolchain_version, install_prefix):
    toolchains = get_toolchains(os.path.join(tempfile.gettempdir(), 'dlcache'))

    toolchain = toolchains.get(toolchain_version)

    if toolchain is None:
        raise NordicSetupException(f'Toolchain {toolchain_version} not found.')

    env = toolchain.setup(install_prefix)
    retval = ''

    if os.name == 'posix':
        for var in env:
            retval += f'export {var}{os.linesep}'

    if os.name == 'nt':
        for var in env:
            retval += f'set {var}{os.linesep}'

    return retval

if __name__ == "__main__":
    logging.basicConfig(level=logging.ERROR)
    parser = argparse.ArgumentParser()
    parser.add_argument('--prefix', help="Install prefix")
    parser.add_argument('--tcversion', help="Toolchain version")
    parser.add_argument('-e', '--env', action='store_true')
    args = parser.parse_args()

    try:
        env = setup_nordic_toolchain(args.tcversion, args.prefix)

        if args.env:
            print(env)
        else:
            logging.debug(
                'Successfully setup toolchain version %s in %s',
                args.tcversion,
                args.prefix
            )
    except NordicSetupException as setup_exception:
        logging.error(setup_exception.message)
