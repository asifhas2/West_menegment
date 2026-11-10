@echo off
echo ========================================================
echo Compiling Community Smart Waste Management System (C-WMS)
echo ========================================================

set GCC_PATH="C:\Program Files\CodeBlocks\MinGW\bin\gcc.exe"

if exist waste_management_old.exe del /f /q waste_management_old.exe >nul 2>&1
if exist waste_management.exe (
    del /f /q waste_management.exe >nul 2>&1
    if exist waste_management.exe ren waste_management.exe waste_management_old.exe >nul 2>&1
)

if exist %GCC_PATH% (
    %GCC_PATH% -Wall -Wextra -Wl,--stack,16777216 -Iheader_file main_c_code/main.c main_c_code/user.c main_c_code/collector.c main_c_code/admin.c main_c_code/data.c main_c_code/utils.c -o waste_management.exe
) else (
    gcc -Wall -Wextra -Wl,--stack,16777216 -Iheader_file main_c_code/main.c main_c_code/user.c main_c_code/collector.c main_c_code/admin.c main_c_code/data.c main_c_code/utils.c -o waste_management.exe
)

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Compilation completed successfully!
    echo Output executable: waste_management.exe
) else (
    echo [ERROR] Compilation failed. Please check errors above.
)
