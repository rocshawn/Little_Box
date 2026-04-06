@echo off
setlocal

:: Kill the process if it's running to avoid file locking
echo Terminating any active Little Box instances...
taskkill /F /IM Little_Box.exe /T 2>NUL

echo Starting Release x64 build...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" /p:Configuration=Release /p:Platform=x64 Little_Box.sln

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Check the output above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] Build completed successfully.
echo Output: e:\Files\Code\Little_Box\x64\Release\Little_Box.exe
pause
