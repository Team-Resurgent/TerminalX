# Run this script after closing Cursor/IDE (e.g. from PowerShell or right-click Run with PowerShell).
# From repo root: d:\Git\DOSXbox

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$oldFolder = Join-Path $root "DOSXbox"
$newFolder = Join-Path $root "TerminalX"
$slnPath = Join-Path $root "TerminalX.sln"

if (-not (Test-Path $oldFolder)) {
    Write-Host "DOSXbox folder not found (already renamed?)."
    exit 0
}
if (Test-Path $newFolder) {
    Write-Host "TerminalX folder already exists. Remove it first if you want to re-run."
    exit 1
}

Rename-Item -Path $oldFolder -NewName "TerminalX" -Force
Write-Host "Renamed DOSXbox to TerminalX."

$sln = Get-Content $slnPath -Raw
$sln = $sln -replace 'DOSXbox\\TerminalX\.vcproj', 'TerminalX\TerminalX.vcproj'
Set-Content -Path $slnPath -Value $sln -NoNewline
Write-Host "Updated TerminalX.sln project path."
Write-Host "Done. Reopen the project in Cursor using folder: $newFolder"
