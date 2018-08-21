@ECHO OFF
SETLOCAL
SET startpath=%cd%
SET scriptpath=%~dp0
SET rootpath=%scriptpath%..\..

REM Get command line argument
IF "%1"=="" (SET CONN_VERSION=0.0.0
) ELSE (SET CONN_VERSION=%1)

ECHO ON

@ECHO Run bootstrap script
cd %scriptpath%
"c:\program files\git\bin\bash.exe" bootstrap_sd_api_v2.sh

@ECHO Download SDK 12 to get newer softdevice version
"c:\program files\git\bin\bash.exe" sdk_download.sh

@ECHO Workaround to reduce path length before build
cd %rootpath%\sdk
rename nRF5_SDK_11.0.0_89a8197 s11
rename nRF5_SDK_12.1.0_0d23e2a s121

@ECHO Compile
cd %rootpath%\sdk\s11\examples\ble_central_and_peripheral\ble_connectivity\pca10028\ser_s130_hci\arm5_no_packs

\Keil_v5\UV4\UV4.exe -b ble_connectivity_s130_hci_pca10028.uvprojx -j0 -o log_1m.txt

@ECHO Merge hex
"c:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\mergehex.exe" -m _build\nrf51422_xxac_s130.hex %rootpath%\sdk\s121\components\softdevice\s130\hex\s130_nrf51_2.0.1_softdevice.hex -o %rootpath%\sdk\connectivity_%CONN_VERSION%_1m_with_s130_2.0.1.hex

@ECHO String replace baudrate
"c:\program files\git\bin\bash.exe" -c "sed -i -e 's/UART_BAUDRATE_BAUDRATE_Baud1M$/UART_BAUDRATE_BAUDRATE_Baud115200/' ../../../../../../../s11/components/serialization/common/ser_config.h"

@ECHO Compile
\Keil_v5\UV4\UV4.exe -b ble_connectivity_s130_hci_pca10028.uvprojx -j0 -o log_115k2.txt

@ECHO Merge hex
"c:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\mergehex.exe" -m _build\nrf51422_xxac_s130.hex %rootpath%\sdk\s121\components\softdevice\s130\hex\s130_nrf51_2.0.1_softdevice.hex -o %rootpath%\sdk\connectivity_%CONN_VERSION%_115k2_with_s130_2.0.1.hex

@ECHO Checking that the output files exist
@IF NOT EXIST %rootpath%\sdk\connectivity_%CONN_VERSION%_1m_with_s130_2.0.1.hex EXIT /b 1
@IF NOT EXIST %rootpath%\sdk\connectivity_%CONN_VERSION%_115k2_with_s130_2.0.1.hex EXIT /b 1
@ECHO Success