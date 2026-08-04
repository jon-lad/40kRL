# run-tests.ps1 — Build the test project and run tests with optional tag filter.
# Usage: .\run-tests.ps1 [TagFilter]
# Examples:
#   .\run-tests.ps1                    # run all tests
#   .\run-tests.ps1 "[action-system]"  # run only action-system tagged tests
param(
    [string]$TagFilter = ""
)

$ErrorActionPreference = "Stop"

# Build
powershell -ExecutionPolicy Bypass -File "$PSScriptRoot\build.ps1" "Tests\40kRL_Tests.vcxproj" Debug x64
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Run
$exe = "$PSScriptRoot\x64\Debug\40kRL_Tests.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Test executable not found at $exe"
    exit 1
}

if ($TagFilter) {
    & $exe $TagFilter
} else {
    & $exe
}
exit $LASTEXITCODE
