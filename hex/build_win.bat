@ECHO OFF
SETLOCAL

@IF EXIST sd_api_v2 CALL :run_build 2
@IF EXIST sd_api_v3 CALL :run_build 3
@IF EXIST sd_api_v5 CALL :run_build 5
@IF EXIST sd_api_v6 CALL :run_build 6

GOTO :EOF

:run_build
@cd sd_api_v%~1 || GOTO :error
@build_v%~1_win.bat || GOTO :error
EXIT /b %errorlevel%

GOTO :EOF

:error
ECHO Command failed with error code %errorlevel%.
EXIT /b %errorlevel%
