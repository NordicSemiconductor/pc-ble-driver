#!/bin/bash
# 
# Download an patch the nRF5 SDK to compile the connectivity application.
#
# The link of the SDK to download can be configured. It will be downloaded and
# extracted to the configured folder. A custom patch file can be applied.
#
# All the paths provided as arguments are relative to the location of this script.
# Usage: bootstrap -l SDK link -d destination folder -p patch file
# 
# Adapted from 'https://github.com/NordicSemiconductor/nrf5-sdk-for-eddystone'.
# Version 0.5

ABS_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# SDK download link (as zip file, full URL with extension)
function set_sdk_link () {
    SDK_LINK=$1

    SDK_FILE=${SDK_LINK##*/}  # SDK file name with extension
    SDK_NAME=${SDK_FILE%.zip} # SDK folder name without extension
}

# Configuration of the destination folder (relative path from this script)
function set_dl_location () {
    DL_LOCATION="$ABS_PATH/$1"
}

# Configuration of the patch file name (relative path from this script)
function set_patch_file () {
    PATCH_FILE="$ABS_PATH/$1"
}

function usage () {
    printf "Usage: bootstrap [[-l SDK link -d destination folder -p patch file ] | [-h]]\r\n"
}

# Display a fatal error and exit
function fatal () {
    printf "\e[1;31m[ERROR] $1\e[0;0m"
    echo
    exit
}

# Check if the required program are available
function check_requirements () {
    command -v unzip >/dev/null 2>&1 || { fatal "Unzip is not available"; }
    command -v curl >/dev/null 2>&1 || { fatal "Curl is not available"; }
    command -v git >/dev/null 2>&1 || { fatal "Git is not available"; }
    command -v patch >/dev/null 2>&1 || { fatal "Patch is not available"; }
}

# Check if the required parameters have been configured
function check_config () {
    if [[ -z "${SDK_LINK}" ]]; then
        fatal "Download SDK link has not been set"
    fi

    if [[ -z "${DL_LOCATION}" ]]; then
        fatal "Download location has not been set"
    fi

    if [[ -z "${PATCH_FILE}" ]]; then
        fatal "Patch file has not been set"
    fi
}


# Check if the SDK folder already exist
function sdk_exists () {
    if [[ -d $DL_LOCATION && -d $DL_LOCATION/$SDK_NAME ]]; then
        # Erase the existing SDK folder if needed
        echo "> SDK folder already available"
        read -p "> Do you want to erase the existing SDK folder [Y/n]? " -n 1 -r
        echo ""

        if [[ $REPLY =~ ^[Y]$ ]]; then
            echo "> Deleting existing SDK folder..."
            rm -rf $DL_LOCATION/$SDK_NAME
        else
            return 0 # Exists
        fi
    fi

    # No SDK folder available
    return 1
}

# Patch the downloaded SDK in order to compile the connectivity application
function sdk_patch () {
    echo "> Applying SDK patch..."

    # Apply the patch from the base nRF SDK folder (remove the first portion of the path)
    # FIXME: check if the patch has been already applied
    patch -d $DL_LOCATION/$SDK_NAME/ -p1 -s --ignore-whitespace -i $PATCH_FILE

    err_code=$?
    if [ "$err_code" != "0" ]; then
        # The patch has been probably already applied
        fatal "> Patch does not apply"
    fi
}

# Download and patch the SDK. Check if it is already available
function sdk_download () {
    # First check if the SDK already exist
    if sdk_exists $1; then
        # SDK folder already available (assume patched)
        return 0
    fi

    if [[ -z "${SDK_NAME}" ]]; then
        fatal "Invalid SDK link"
    fi

    # Create the destination folder and download the SDK
    echo "> Downloading nRF SDK..."

    mkdir -p $DL_LOCATION

    curl --progress-bar -o $DL_LOCATION/$SDK_FILE $SDK_LINK

    err_code=$?
    if [ "$err_code" != "0" ]; then
        fatal "Could not download SDK from '${SDK_LINK}'"
    fi

    echo "> Unzipping SDK..."
    unzip -q $DL_LOCATION/$SDK_FILE -d $DL_LOCATION/$SDK_NAME

    err_code=$?
    if [ "$err_code" != "0" ]; then
        fatal "Could not unzip the SDK file"
    fi

    echo "> Clean up. Removing SDK zip file..."
    rm $DL_LOCATION/$SDK_FILE

    err_code=$?
    if [ "$err_code" != "0" ]; then
        fatal "Could not remove the SDK zip file"
    fi

    # FIXME: unused files from the modified SDK should be deleted
    # Keep only the components and the connectivity application ?
}

function main() {

    while [ "$1" != "" ]; do
        case $1 in
            -l | --link )  shift
                           set_sdk_link $1
                           ;;
            -d | --dest )  shift
                           set_dl_location $1
                           ;;
            -p | --patch ) shift
                           set_patch_file $1
                           ;;
            -h | --help )  usage
                           exit
                           ;;
            * )            usage
                           exit 1
        esac
        shift
    done

    check_requirements
    check_config
    sdk_download
    sdk_patch

    echo "> SDK ready to use. Exit."
    exit
}

main "$@"
