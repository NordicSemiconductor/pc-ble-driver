#!/usr/bin/env python

import pycurl
import certifi
from urllib.parse import urlparse


class DownloadCache:
    def __init__(self, path):
        self.path = path

    def exists(self, filename, sha512):
        pass

    def path(self, filename, sha512):
        pass


class Downloader:
    def __init__(self, download_url, download_cache):
        self.downloadCache = downloadCache

    def download(self, download_url, download_filename=None):
        # TODO: extract download filename
        url = download_url

        filename = None

        if download_filename is None:
            url = urlparse(download_url)
            filename = url.path.split("/")[-1]
        else:
            filename = download_filename

        if self.downloadCache and self.downloadCache.exists(filename):
            return self.downloadCache.path(filename)

        # Download SDK
        with open(filename, "wb") as f:
            c = pycurl.Curl()
            c.setopt(c.URL, url)
            c.setopt(c.WRITEDATA, f)
            c.setopt(c.FOLLOWLOCATION, True)
            c.setopt(c.MAXREDIRS, 5)
            c.setopt(c.CAINFO, certifi.where())
            c.perform()
            c.close()


class DownloadInfo:
    def __init__(self, download_url, download_filename, download_sha512):
        self.download_url = download_url
        self.download_filename = download_filename
        self.download_sha512 = download_sha512


class Compiler:
    def __init__(self, downloadInfo):
        self.downloadInfo = downloadInfo
        self.cacheDirectory = None

    def download(self):
        if self.downloadInfo is None:
            raise Exception("Not download info specified for compiler")

    def install(self, prefix):
        pass


class Sdk:
    def __init__(self, version, compiler):
        self.version = version
        self.compiler = compiler

    def setup(self, install_prefix, download_cache):
        if self.compiler is None:
            self.compiler.download(download_cache)
            self.compiler.install(install_prefix)

        # TODO: add compiler env variables in return


def setup_nordic_sdk(sdk_version, install_prefix):
    download_cache = DownloadCache("C:\\tmp\\dlcache")

    sdks = {
        "nRF5_SDK_v11": Sdk("nRF5_SDK_v11", Compiler(DownloadInfo("", "", ""))),
        "nRF5_SDK_v12": Sdk("nRF5_SDK_v12", Compiler(DownloadInfo("", "", ""))),
        "nRF5_SDK_v13": Sdk("nRF5_SDK_v13", Compiler(DownloadInfo("", "", ""))),
        "nRF5_SDK_v14": Sdk("nRF5_SDK_v14", Compiler(DownloadInfo("", "", ""))),
        "nRF5_SDK_v15": Sdk("nRF5_SDK_v15", Compiler(DownloadInfo("", "", ""))),
        "nRF5_SDK_v16": Sdk("nRF5_SDK_v16", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v4": Sdk("nRF51_SDK_v4", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v5": Sdk("nRF51_SDK_v5", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v6": Sdk("nRF51_SDK_v6", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v7": Sdk("nRF51_SDK_v7", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v8": Sdk("nRF51_SDK_v8", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v9": Sdk("nRF51_SDK_v9", Compiler(DownloadInfo("", "", ""))),
        "nRF51_SDK_v10": Sdk("nRF51_SDK_v10", Compiler(DownloadInfo("", "", ""))),
        "ncs-0.3.0": Sdk("ncs-0.3.0", Compiler(DownloadInfo("", "", ""))),
        "ncs-0.4.0": Sdk("ncs-0.4.0", Compiler(DownloadInfo("", "", ""))),
        "ncs-1.0.0": Sdk("ncs-1.0.0", Compiler(DownloadInfo("", "", ""))),
        "ncs-1.1.0": Sdk("ncs-1.1.0", Compiler(DownloadInfo("", "", ""))),
        "ncs-1.2.0": Sdk("ncs-1.2.0", Compiler(DownloadInfo("", "", ""))),
    }

    sdk = sdks[sdk_version]

    if sdks[sdk_version] is None:
        raise Exception(f"SDK version {sdk_version} not found")

    sdk.setup(install_prefix, download_cache)


if __name__ == "__main__":
    setup_nordic_sdk("nRF5_SDK_v16", "c:\\tmp")
