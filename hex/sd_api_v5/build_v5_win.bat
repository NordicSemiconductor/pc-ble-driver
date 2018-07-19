@ECHO OFF
SETLOCAL
SET startpath=%cd%
SET scriptpath=%~dp0
SET rootpath=%scriptpath%..\..

REM Get command line argument
IF "%1"=="" (SET CONN_VERSION=0.0.0
) ELSE (SET CONN_VERSION=%1)

REM Environment variables that is changed between SDK versions
@SET SDK_NAME=nRF5_SDK_15.0.0_a53641a
@SET SOFTDEVICE_MAJOR=5
@SET SOFTDEVICE_MINOR=0
@SET SOFTDEVICE_PATCH=0

REM Environment variables that are below that are mostly based on the ones above
REM Do not know why a shortened name is used
@SET TEMP_NAME=s150
@SET SOFTDEVICE_PATH="%rootpath%\sdk\%TEMP_NAME%\components\softdevice\s132v%SOFTDEVICE_MAJOR%\hex\s132_nrf52_%SOFTDEVICE_MAJOR%.%SOFTDEVICE_MINOR%.%SOFTDEVICE_PATCH%_softdevice.hex"
@SET MERGED_HEX_FILENAME_PCA10040_1m="%rootpath%\sdk\connectivity_%CONN_VERSION%_1m_with_s132_%SOFTDEVICE_MAJOR%.%SOFTDEVICE_MINOR%.hex"
@SET MERGED_HEX_FILENAME_PCA10040_115k2="%rootpath%\sdk\connectivity_%CONN_VERSION%_115k2_with_s132_%SOFTDEVICE_MAJOR%.%SOFTDEVICE_MINOR%.hex"
@SET MERGED_HEX_FILENAME_PCA10059_usb="%rootpath%\sdk\connectivity_%CONN_VERSION%_usb_with_s132_%SOFTDEVICE_MAJOR%.%SOFTDEVICE_MINOR%.hex"

@SET BASH_EXE="c:\program files\git\bin\bash.exe"
@SET MERGE_HEX="c:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\mergehex.exe"
@SET UV_EXE="\Keil_v5\UV4\UV4.exe"
@SET UV_PROJECT_FILE_PCA10040="ble_connectivity_s132v%SOFTDEVICE_MAJOR%_hci_pca10040.uvprojx"
@SET UV_PROJECT_FILE_PCA10059="ble_connectivity_s132v%SOFTDEVICE_MAJOR%_usb_hci_pca10059.uvprojx"

@ECHO Starting build of BLE Connectivity firmware with SDK %SDK_NAME%
cd "%scriptpath%" || GOTO :error
"c:\program files\git\bin\bash.exe" bootstrap_sd_api_v%SOFTDEVICE_MAJOR%.sh || GOTO :error

@ECHO Workaround to reduce path length before build
cd %rootpath%\sdk || GOTO :error
rename %SDK_NAME% %TEMP_NAME% || GOTO :error

REM ##############
REM #  PCA10040  #
REM ##############
@ECHO.
@ECHO Compiling for PCA10040, 1M baudrate
@cd %rootpath%\sdk\%TEMP_NAME%\examples\connectivity\ble_connectivity\pca10040\ser_s132v%SOFTDEVICE_MAJOR%_hci\arm5_no_packs || GOTO :error
%UV_EXE% -b %UV_PROJECT_FILE_PCA10040% -j0 -o pca10040_log_1m.txt

REM Warnings are expected, UV return codes: http://www.keil.com/support/man/docs/uv4/uv4_commandline.htm
@if %ERRORLEVEL% GTR 1 GOTO :error

@ECHO Merge hex SoftDevice and application.
%MERGE_HEX% -m _build\nrf52832_xxaa.hex %SOFTDEVICE_PATH% -o %MERGED_HEX_FILENAME_PCA10040_1m% || GOTO :error

@ECHO String replace baudrate.
%BASH_EXE% -c "sed -i -e 's/SER_PHY_UART_BAUDRATE_VAL       1000000$/SER_PHY_UART_BAUDRATE_VAL       115200/' ../../../../../../../%TEMP_NAME%/components/serialization/common/ser_config.h" || GOTO :error

@ECHO.
@ECHO Compiling for PCA10040, 115k2 baudrate
%UV_EXE% -b %UV_PROJECT_FILE_PCA10040% -j0 -o pca10040_log_115k2.txt
@if %ERRORLEVEL% GTR 1 GOTO :error

@ECHO Merge hex SoftDevice and application.
%MERGE_HEX% -m _build\nrf52832_xxaa.hex %SOFTDEVICE_PATH% -o %MERGED_HEX_FILENAME_PCA10040_115k2% || GOTO :error

REM ##############
REM #  PCA10059  #
REM ##############
@ECHO.
@ECHO Compiling for PCA10059, USB
@SET PROJECT_ROOT_PATH="%rootpath%\sdk\%TEMP_NAME%\examples\connectivity\ble_connectivity\pca10059\ser_s132v%SOFTDEVICE_MAJOR%_usb_hci\arm5_no_packs"
@IF NOT EXIST %PROJECT_ROOT_PATH% CALL :skipping "PCA10059, USB"
@IF %ERRORLEVEL%==1 GOTO :summary
@cd %PROJECT_ROOT_PATH% || GOTO :error
%UV_EXE% -b %UV_PROJECT_FILE_PCA10059% -j0 -o pca10059_log_usb.txt

REM Warnings are expected, UV return codes: http://www.keil.com/support/man/docs/uv4/uv4_commandline.htm
@if %ERRORLEVEL% GTR 1 GOTO :error

@ECHO Merge hex SoftDevice and application.
%MERGE_HEX% -m _build\nrf52832_xxaa.hex %SOFTDEVICE_PATH% -o %MERGED_HEX_FILENAME_PCA10059_usb% || GOTO :error

:summary
@ECHO.
@ECHO Connectivity firmwares compiled and merged:
@IF EXIST %MERGED_HEX_FILENAME_PCA10040_1m% ECHO %MERGED_HEX_FILENAME_PCA10040_1m%
@IF EXIST %MERGED_HEX_FILENAME_PCA10040_115k2% ECHO %MERGED_HEX_FILENAME_PCA10040_115k2%
@IF EXIST %MERGED_HEX_FILENAME_PCA10059_usb% ECHO %MERGED_HEX_FILENAME_PCA10059_usb%

GOTO :EOF

:error
ECHO Command failed with error code %errorlevel%.
EXIT /b %errorlevel%

:skipping
ECHO Skipping compilation of %~1 since the project file does not exist
EXIT /b 1
