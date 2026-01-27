<#
.SYNOPSIS
    Packages the Photobooth Backend into a SINGLE EXE using 7-Zip SFX
.DESCRIPTION
    1. Checks if the 'dist' folder (created by package_release.ps1) exists.
    2. Downloads the 7-Zip SFX module (7zSD.sfx) if missing.
    3. Configures the SFX header to auto-run the watchdog bat or exe.
    4. Compresses the dist folder and merges it with the SFX header.
#>

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
$DistDir = "$ProjectRoot/dist/PhotoboothPro_Server"
$OutputDir = "$ProjectRoot/dist"
$FinalExe = "$OutputDir/PhotoboothServer_Setup.exe"

# 1. Validation
if (-not (Test-Path $DistDir)) {
    Write-Error "Dist directory not found! Please run 'package_release.ps1' first."
    exit 1
}

# 2. Check for 7-Zip
$7z = "C:\Program Files\7-Zip\7z.exe"
if (-not (Test-Path $7z)) {
    $7z = "C:\Program Files (x86)\7-Zip\7z.exe"
}
if (-not (Test-Path $7z)) {
    Write-Error "7-Zip not found! Please install 7-Zip to create an SFX."
    exit 1
}

Write-Host ">>> [1/3] Compressing files..." -ForegroundColor Cyan
Set-Location $OutputDir
# Remove old artifacts
if (Test-Path "payload.7z") { Remove-Item "payload.7z" }
if (Test-Path $FinalExe) { Remove-Item $FinalExe }

# Zip the CONTENTS of the dist folder, not the folder itself, so extraction is cleaner
& $7z a "payload.7z" "$DistDir\*" -mx9

# 3. Create SFX Config
Write-Host ">>> [2/3] Configuring SFX Header..." -ForegroundColor Cyan
$SfxConfig = @"
;!@Install@!UTF-8!
Title="Photobooth Pro Server"
RunProgram="run_backend_watchdog.bat"
GUIMode="1"
ExtractTitle="Initializing Photobooth Server..."
ExtractPath="%TEMP%\PhotoboothServer"
;!@InstallEnd@!
"@
Set-Content -Path "config.txt" -Value $SfxConfig -Encoding UTF8

# 4. Merge to create EXE
Write-Host ">>> [3/3] Merging into Single EXE..." -ForegroundColor Cyan
# We need the 7zSD.sfx module. It usually comes with 7-Zip via '7z-extra' or similar. 
# Typical path: C:\Program Files\7-Zip\7zSD.sfx
$SfxModule = "C:\Program Files\7-Zip\7zSD.sfx"

if (-not (Test-Path $SfxModule)) {
    Write-Warning "Standard SFX module not found at $SfxModule. Searching..."
    # Fallback search or download could go here, but for now we error out or try generic
    Write-Error "7zSD.sfx module missing. Please install '7-Zip Extra' or ensure 7zSD.sfx is in 7-Zip folder."
    exit 1
}

# Combine: SFX_Module + Config + Archive = EXE
cmd /c "copy /b `"$SfxModule`" + config.txt + payload.7z `"$FinalExe`""

# Cleanup
Remove-Item "payload.7z"
Remove-Item "config.txt"

Write-Host ">>> DONE! Single Executable created at: $FinalExe" -ForegroundColor Green
Write-Host ">>> NOTE: This exe extracts to %TEMP% and runs. PROS: Single file. CONS: Slower startup." -ForegroundColor Yellow
