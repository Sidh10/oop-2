@echo off
rem Opens the Quant Strategy Advisor menu. Double-click this file, or run it from cmd.
rem Extra arguments are passed through to the program.
rem The program reads and writes data/ and reports/ relative to this folder,
rem so we always start from the folder this file lives in.
cd /d "%~dp0"

if not exist "build\quant_advisor.exe" (
    echo build\quant_advisor.exe was not found.
    echo Build the project first - see README.md.
    echo.
    pause
    exit /b 1
)

"build\quant_advisor.exe" %*

echo.
pause
