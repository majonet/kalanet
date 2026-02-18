@echo off
chcp 65001 >nul
echo ==========================================
echo KalaNet Build Script for Windows
echo ==========================================

:: Find Qt installation
if exist "C:\Qt.5.0\mingw_64in\qmake.exe" (
    set QT_DIR=C:\Qt.5.0\mingw_64in
) else if exist "C:\Qt.4.0\mingw_64in\qmake.exe" (
    set QT_DIR=C:\Qt.4.0\mingw_64in
) else if exist "C:\Qt.15.2\mingw81_64in\qmake.exe" (
    set QT_DIR=C:\Qt.15.2\mingw81_64in
) else (
    echo Error: Qt installation not found!
    echo Please install Qt from https://www.qt.io/download
    pause
    exit /b 1
)

echo Found Qt at: %QT_DIR%
set PATH=%QT_DIR%;%PATH%

:: Clean previous build
if exist build rmdir /s /q build
mkdir build
cd build

:: Run qmake
echo Running qmake...
qmake ..\KalaNet.pro
if errorlevel 1 (
    echo qmake failed!
    pause
    exit /b 1
)

:: Build
echo Building project...
mingw32-make -j4
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ==========================================
echo Build successful!
echo Executable: buildelease\KalaNet.exe
echo ==========================================
echo.
echo Default login: admin / Admin123
echo.
pause
