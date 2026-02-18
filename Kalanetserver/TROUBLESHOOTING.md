# KalaNet Build Troubleshooting Guide

## Common Errors and Solutions

### Error: "[Makefile.Debug:93: debug/KalaNet.exe] Error 1" (Linker Error)

This is a linker error. Here are the solutions:

#### Solution 1: Clean Build
1. Delete the `build` folder completely
2. Delete any `.pro.user` files
3. Rebuild from scratch

#### Solution 2: Use Qt Creator (Recommended)
The easiest way to build:
1. Open Qt Creator
2. File → Open Project → Select `KalaNet.pro`
3. Build → Run qmake (Ctrl+Shift+B)
4. Build → Build Project (Ctrl+B)

#### Solution 3: Manual qmake and make
```bash
# Clean everything
cd KalaNet
rm -rf build
mkdir build
cd build

# Run qmake
qmake ../KalaNet.pro

# For Windows (MinGW):
mingw32-make clean
mingw32-make

# For Linux/macOS:
make clean
make
```

#### Solution 4: Check Qt Installation
Make sure your Qt environment is set up correctly:

**Windows:**
```cmd
set PATH=C:\Qt.15.2\mingw81_64in;%PATH%
qmake --version
```

**Linux:**
```bash
export PATH=/usr/lib/qt5/bin:$PATH
qmake --version
```

### Error: "undefined reference to vtable"
This means moc (Meta-Object Compiler) didn't run. Solution:
1. Run `qmake -tp vc` (Windows) or just `qmake` (Linux/Mac)
2. Make sure all headers with Q_OBJECT are listed in HEADERS in .pro file

### Error: "cannot find -lQt5Core" or similar
Qt libraries not found. Solution:
1. Add Qt bin directory to PATH
2. Or use Qt Creator which sets up environment automatically

### Error: "No rule to make target"
The Makefile is corrupted. Solution:
1. Delete build directory
2. Run qmake again
3. Run make again

## Building Step by Step

### Windows with MinGW:
```cmd
cd C:\path	o\KalaNet

:: Set Qt path (adjust version as needed)
set PATH=C:\Qt.15.2\mingw81_64in;%PATH%

:: Create build directory
mkdir build
cd build

:: Generate Makefile
qmake ..\KalaNet.pro

:: Build
mingw32-make -j4

:: Run
release\KalaNet.exe
```

### Linux:
```bash
cd /path/to/KalaNet

# Install Qt if needed
sudo apt-get install qt5-default build-essential

# Build
mkdir build && cd build
qmake ..
make -j4

# Run
./KalaNet
```

### macOS:
```bash
cd /path/to/KalaNet

# Install Qt (using Homebrew)
brew install qt@5

# Build
mkdir build && cd build
qmake ..
make -j4

# Run
./KalaNet.app/Contents/MacOS/KalaNet
```

## Verification Checklist

Before building, verify:
- [ ] Qt is installed (5.15+ or 6.x)
- [ ] qmake is in PATH
- [ ] Compiler is installed (g++, mingw, or clang)
- [ ] All source files are present
- [ ] No previous build artifacts (clean build)

## Still Having Issues?

1. **Use Qt Creator** - It handles all the build configuration automatically
2. **Check the Compile Output** - Look for the first error, not the last one
3. **Verify File Encoding** - Files should be UTF-8
4. **Check Line Endings** - Use LF (Unix) or CRLF (Windows) consistently

## Default Login
Once built successfully:
- Username: `admin`
- Password: `Admin123`
