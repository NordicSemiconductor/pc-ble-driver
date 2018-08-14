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
"c:\program files\git\bin\bash.exe" bootstrap_sd_api_v3.sh

@ECHO Workaround to reduce path length before build
cd %rootpath%\sdk
rename nRF5_SDK_12.2.0_f012efa s122

@ECHO Compile
@cd %rootpath%\sdk\s122\examples\ble_central_and_peripheral\ble_connectivity\pca10040\ser_s132_hci\arm5_no_packs
\Keil_v5\UV4\UV4.exe -b ble_connectivity_s132_hci_pca10040.uvprojx -j0 -o log_1m.txt

@ECHO Merge hex
"c:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\mergehex.exe" -m _build\nrf52832_xxaa.hex %rootpath%\hex\sd_api_v3\s132_nrf52_3.1.0_softdevice.hex -o %rootpath%\sdk\connectivity_%CONN_VERSION%_1m_with_s132_3.1.hex

@ECHO String replace baudrate
"c:\program files\git\bin\bash.exe" -c "sed -i -e 's/UART_BAUDRATE_BAUDRATE_Baud1M$/UART_BAUDRATE_BAUDRATE_Baud115200/' ../../../../../../../s122/components/serialization/common/ser_config.h"

@ECHO Compile
\Keil_v5\UV4\UV4.exe -b ble_connectivity_s132_hci_pca10040.uvprojx -j0 -o log_115k2.txt

@ECHO Merge hex
"c:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\mergehex.exe" -m _build\nrf52832_xxaa.hex %rootpath%\hex\sd_api_v3\s132_nrf52_3.1.0_softdevice.hex -o %rootpath%\sdk\connectivity_%CONN_VERSION%_115k2_with_s132_3.1.hex

@ECHO Checking that the output files exist
@IF NOT EXIST %rootpath%\sdk\connectivity_%CONN_VERSION%_1m_with_s132_3.1.hex EXIT /b 1
@IF NOT EXIST %rootpath%\sdk\connectivity_%CONN_VERSION%_115k2_with_s132_3.1.hex EXIT /b 1
@ECHO Success