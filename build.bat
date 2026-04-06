@echo off
"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath > vspath.txt
set /p VSPATH=<vspath.txt
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
msbuild Little_Box.vcxproj -v:m -clp:ErrorsOnly;WarningsOnly > build_dump.txt 2>&1
