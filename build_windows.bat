@echo off
echo ============================================================
echo  qmtn (Qt Movie Thumbnailer) Windows Chinese Version Build
echo ============================================================

REM Check if qmake is available
where qmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] qmake was not found in PATH!
    echo Please run this script inside Qt Command Prompt (e.g. Qt 5.15 / Qt 6 MinGW/MSVC Prompt).
    pause
    exit /b 1
)

echo [1/4] Compiling Chinese Translation (zh_CN.ts -^> zh_CN.qm)...
lrelease lang\zh_CN.ts -qm lang\zh_CN.qm
if %errorlevel% neq 0 (
    echo [WARNING] lrelease failed or not found, using precompiled zh_CN.ts
)

echo [2/4] Generating Makefile with qmake...
qmake src\mtngui.pro CONFIG+=release

echo [3/4] Compiling source code...
where mingw32-make >nul 2>nul
if %errorlevel% equ 0 (
    mingw32-make -j4
) else (
    nmake
)

if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed!
    pause
    exit /b 1
)

echo [4/4] Deploying Qt libraries with windeployqt...
if not exist "dist" mkdir dist
copy release\qmtn.exe dist\
windeployqt --release dist\qmtn.exe

echo ============================================================
echo  Build Completed Successfully!
echo  Executable files are located in directory: dist\
echo ============================================================
pause
