@echo off
chcp 65001 >nul
echo ==========================================
echo KalaNet Build Script for Windows
echo ==========================================
echo.

:: Check if we're in the right directory
if not exist "KalaNet.pro" (
    echo Error: KalaNet.pro not found!
    echo Please run this script from the KalaNet directory.
    pause
    exit /b 1
)

:: Find Qt installation
echo Searching for Qt installation...

set QT_PATH=

:: Check common Qt locations
if exist "C:\Qt\6.5.3\mingw_64\bin\qmake.exe" (
    set QT_PATH=C:\Qt\6.5.3\mingw_64\bin
) else if exist "C:\Qt\6.4.3\mingw_64\bin\qmake.exe" (
    set QT_PATH=C:\Qt\6.4.3\mingw_64\bin
) else if exist "C:\Qt\6.2.4\mingw_64\bin\qmake.exe" (
    set QT_PATH=C:\Qt\6.2.4\mingw_64\bin
) else if exist "C:\Qt\5.15.2\mingw81_64\bin\qmake.exe" (
    set QT_PATH=C:\Qt\5.15.2\mingw81_64\bin
) else if exist "C:\Qt\5.15.0\mingw81_64\bin\qmake.exe" (
    set QT_PATH=C:\Qt\5.15.0\mingw81_64\bin
)

if "%QT_PATH%"=="" (
    echo Error: Qt installation not found in common locations!
    echo Please install Qt from https://www.qt.io/download
    echo Make sure to install MinGW compiler with Qt.
    pause
    exit /b 1
)

echo Found Qt at: %QT_PATH%
set PATH=%QT_PATH%;%PATH%

:: Verify qmake works
qmake --version >nul 2>&1
if errorlevel 1 (
    echo Error: qmake not working properly!
    pause
    exit /b 1
)

echo.
echo ==========================================
echo Cleaning previous build...
echo ==========================================

:: Clean previous build
if exist build (
    rmdir /s /q build
    echo Cleaned old build directory
)

:: Clean other generated files
del /f /q *.pro.user 2>nul
del /f /q Makefile* 2>nul
del /f /q .qmake.stash 2>nul

echo.
echo ==========================================
echo Creating build directory...
echo ==========================================

mkdir build
cd build

echo.
echo ==========================================
echo Running qmake...
echo ==========================================

qmake ..\KalaNet.pro
if errorlevel 1 (
    echo.
    echo ERROR: qmake failed!
    echo This usually means:
    echo 1. Qt is not installed correctly
    echo 2. The .pro file has syntax errors
    echo 3. Missing Qt modules
    cd ..
    pause
    exit /b 1
)

echo.
echo ==========================================
echo Building project...
echo This may take a few minutes...
echo ==========================================

mingw32-make -j4
if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    echo Common causes:
    echo 1. Syntax errors in source code
    echo 2. Missing header files
    echo 3. Linker errors - missing libraries
    echo.
    echo Please check the error messages above.
    cd ..
    pause
    exit /b 1
)

echo.
echo ==========================================
echo Build successful!
echo ==========================================
echo.
echo Executable location:
echo   debug\KalaNet.exe   (Debug build)
echo   release\KalaNet.exe (Release build)
echo.
echo Default login:
echo   Username: admin
echo   Password: Admin123
echo.
echo To run the program:
echo   debug\KalaNet.exe
echo.
pause
