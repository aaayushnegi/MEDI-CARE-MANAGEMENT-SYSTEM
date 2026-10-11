@echo off
title MediCare — Medical Management System
color 0B
echo ==============================================================
echo   MediCare -- Medical Management System (BCA 1st Year)
echo ==============================================================
echo [1/2] Compiling C Backend with GCC...
gcc -Wall -Wextra -o backend\server.exe backend\main.c backend\patient.c backend\doctor.c backend\appointment.c backend\records.c backend\prescription.c backend\billing.c backend\beds.c backend\common.c -lws2_32

if %ERRORLEVEL% NEQ 0 (
    color 0C
    echo [ERROR] Compilation failed! Ensure MinGW GCC is installed.
    pause
    exit /b %ERRORLEVEL%
)

echo [2/2] Compilation Successful! Launching MediCare Server...
echo.
echo Launching your default browser to http://localhost:8080/ ...
start http://localhost:8080/
echo.
backend\server.exe
pause
