# build.ps1 — Wrapper for MSBuild that resolves the VS path automatically.
# Usage: .\build.ps1 [project] [Configuration] [Platform]
# Defaults: Tests/40kRL_Tests.vcxproj, Debug, x64
param(
    [string]$Project = "Tests\40kRL_Tests.vcxproj",
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$msbuild = "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"

if (-not (Test-Path $msbuild)) {
    Write-Error "MSBuild not found at $msbuild"
    exit 1
}

& $msbuild $Project /p:Configuration=$Configuration /p:Platform=$Platform /m /nologo /v:minimal
exit $LASTEXITCODE
