# Windows Issues

## /dev/null git apply issue

New version of git is required to solve this issue.

## CreateProcess issue

```
echo  Makefile
CreateProcess(NULL,echo Makefile,...)
process_begin: CreateProcess(NULL, echo Makefile, ...) failed.
Putting child 01DD8B10 (_build) PID 31536216 on the chain.
Live child 01DD8B10 (_build) PID 31536216
make (e=2): The system cannot find the file specified.
Reaping losing child 01DD8B10 PID 31536216
make: *** [Makefile:323: _build] Error 2
Removing child 01DD8B10 PID 31536216 from chain.
```

This issue is due to missing `sh` or `bash` in path.
Adding `sh` or `bash` will fix the issue.

## Long paths issue

```
build   25-Oct-2018 17:17:07      Compiling file: ble_gap_evt_conn.c
build   25-Oct-2018 17:17:07      ../../../../../../components/serialization/connectivity/codecs/ble/serializers/ble_gap_evt_conn.c:46:10: fatal error: conn_ble_gap_sec_keys.h: No such file or directory
build   25-Oct-2018 17:17:07       #include "conn_ble_gap_sec_keys.h"
build   25-Oct-2018 17:17:07                ^~~~~~~~~~~~~~~~~~~~~~~~~
build   25-Oct-2018 17:17:07      compilation terminated.
build   25-Oct-2018 17:17:07      make: *** [../../../../../../components/toolchain/gcc/Makefile.common:272:
```

# Segger Issues

## SEGGER OB sends invalid packet

![USB Analyzer](./segger_ob_usb_analyzer.png)
![Logic Analyzer](./segger_ob_logic_analyzer.png)
