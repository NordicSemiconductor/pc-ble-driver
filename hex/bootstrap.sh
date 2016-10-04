#!/bin/bash
# 
# Download an patch the nRF5 SDK to compile the connectivity application.
# Run this script from the `hex` folder. Download and install the SDK in a tmp folder.
#
# Version 0.1

# SDK download link (as zip file, full URL with extension)
SDK_LINK='https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip'

DL_LOCATION=../tmp/

SDK_FILE=${SDK_LINK##*/}  # SDK file name with extension
SDK_NAME=${SDK_FILE%.zip} # SDK folder name without extension

function fatal () {
	printf "\e[1;31m[ERROR] $1\r\n[ERROR] Press Enter to exit...\e[0;0m"
  read
  exit 1
}

function check() {
	command -v unzip >/dev/null 2>&1 || { fatal "Unzip is not available"; }
	command -v curl >/dev/null 2>&1 || { fatal "Curl is not available"; }
	command -v git >/dev/null 2>&1 || { fatal "Git is not available"; }
}

function sdk_downalod () {
	if [[ -z "${SDK_NAME}" ]]
	then
		fatal "Invalid SDK link"
	fi

	# Create the destination folder or abort if already exist
	if [ -d $DL_LOCATION ]
	then
	  echo "> SDK folder already exist. Skip download..."
	  return 0
	else
	  mkdir $DL_LOCATION
	fi

	echo "> Downloading nRF SDK..."
	curl --progress-bar -o $DL_LOCATION/$SDK_FILE $SDK_LINK

	err_code=$?
	if [ "$err_code" != "0" ]
	then
	  fatal "Could not download SDK from '${SDK_LINK}'"
	fi

	echo "> Unzipping SDK..."
	unzip -q $DL_LOCATION/$SDK_FILE -d $DL_LOCATION/$SDK_NAME

	err_code=$?
	if [ "$err_code" != "0" ]
	then
	  fatal "Could not unzip the SDK file"
	fi

	echo "> Clean up. Removing zip file..."
	rm $DL_LOCATION/$SDK_FILE

	err_code=$?
	if [ "$err_code" != "0" ]
	then
	  fatal "Could not remove the SDK zip file"
	fi
}

function sdk_patch () {
	echo "> Apply patch..."

	# Apply the patch from the SDK root folder
	pwd
	cd $DL_LOCATION/$SDK_NAME/
	pwd
	git apply --stat -p1 -v --ignore-whitespace ../../hex/SD20_SDK11.patch
	# git apply -p1 -v --ignore-whitespace ../../hex/SD20_SDK11.patch

	err_code=$?
	if [ "$err_code" != "0" ]
	then
	  # The patch has been probably already applied
	  echo "> Patch does not apply, Skipping..."
	fi

	echo "> SDK set-up completed"
}

clear
printf "Starting bootstrap script\r\n"
check
sdk_downalod
sdk_patch

printf "\r\n> All done!\r\nPress Enter to exit..."
read

exit 0

