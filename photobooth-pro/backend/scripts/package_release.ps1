<#
.SYNOPSIS
    Compiles and Packages the Photobooth Backend for Production
.DESCRIPTION
    1. Builds the project in Release mode
    2. Creates a 'dist' directory
    3. Copies the Executable, all DLLs, and Configs
    4. Zips it up for easy transfer
#>

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = "$ProjectRoot/build"
$DistDir = "$ProjectRoot/dist/PhotoboothPro_Server"

Write-Host ">>> [1/4] Building Project (Release)..." -ForegroundColor Cyan
Set-Location $ProjectRoot
if (-not (Test-Path $BuildDir)) { mkdir $BuildDir | Out-Null }
cd $BuildDir
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) { Write-Error "Build Failed"; exit 1 }

Write-Host ">>> [2/4] Preparing Distribution Directory..." -ForegroundColor Cyan
if (Test-Path $DistDir) { Remove-Item -Recurse -Force $DistDir }
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
New-Item -ItemType Directory -Force -Path "$DistDir/data" | Out-Null
New-Item -ItemType Directory -Force -Path "$DistDir/logs" | Out-Null

Write-Host ">>> [3/4] Copying Binaries & Dependencies..." -ForegroundColor Cyan
$BinSource = "$BuildDir/bin/Release"

# Copy Executable
Copy-Item "$BinSource/photobooth-server.exe" -Destination $DistDir

# Copy DLLs (EDSDK, OpenCV, etc)
# Note: CMake post-build commands should have already put them in BinSource 
# but we explicitly look for them to be safe.
$Dlls = Get-ChildItem -Path $BinSource -Filter *.dll
foreach ($dll in $Dlls) {
    Copy-Item $dll.FullName -Destination $DistDir
}

# Copy Database if it exists (Empty template or current)
if (Test-Path "$ProjectRoot/photobooth.db") {
    Copy-Item "$ProjectRoot/photobooth.db" -Destination $DistDir
}

Write-Host ">>> [4/4] Creating Launch Scripts..." -ForegroundColor Cyan

# Create a simple Bat launcher that hides console (optional) or restarts on crash
$LauncherContent = @"
@echo off
title Photobooth Backend Watchdog
echo [INFO] Starting Photobooth Server in LOOP mode...
:loop
echo [INFO] Launching server at %time%...
photobooth-server.exe
echo [WARNING] Server crashed or closed! Restarting in 2 seconds...
timeout /t 2 /nobreak >nul
goto loop
"@
Set-Content -Path "$DistDir/run_backend_watchdog.bat" -Value $LauncherContent

# README
$ReadmeContent = @"
PHOTOBOOTH PRO SERVER - PRODUCTION RELEASE
==========================================

SETUP:
1. Ensure Driver for Canon Camera is installed (Standard Windows USB).
2. Connect Camera via USB.
3. Close EOS Utility if it opens automatically.

RUNNING:
- Double click 'run_backend_watchdog.bat' to start the server.
- The server runs on http://localhost:8080 mapping to ::8080 and ws://localhost:8081.

TROUBLESHOOTING:
- Check 'logs/' folder for detailed error reports.
"@
Set-Content -Path "$DistDir/README.txt" -Value $ReadmeContent

Write-Host ">>> SUCCESS! Distribution ready at: $DistDir" -ForegroundColor Green
Write-Host ">>> You can zip the folder '$DistDir' and verify it on another machine." -ForegroundColor Green
