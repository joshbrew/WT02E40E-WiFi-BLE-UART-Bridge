param([switch]$Pristine)
$ErrorActionPreference = 'Stop'
$taskSdk = 'C:\ncs\v3.3.1'
$taskToolchain = 'C:\ncs\toolchains\936afb6332'
$taskApp = $PSScriptRoot
$taskBuild = Join-Path $taskApp 'build\jolteon-routing'
foreach ($entry in (Get-Content (Join-Path $taskToolchain 'environment.json') -Raw | ConvertFrom-Json).env_vars) {
    if ($entry.type -eq 'relative_paths') {
        $value = ($entry.values | ForEach-Object { Join-Path $taskToolchain $_ }) -join ';'
        if ($entry.existing_value_treatment -eq 'prepend_to') {
            $value += ';' + [Environment]::GetEnvironmentVariable($entry.key, 'Process')
        }
    } else { $value = $entry.value }
    [Environment]::SetEnvironmentVariable($entry.key, $value, 'Process')
}
$env:ZEPHYR_BASE = Join-Path $taskSdk 'zephyr'
$taskPython = Join-Path $taskToolchain 'opt\bin\python.exe'
$taskArgs = @('-m','west','build','--sysbuild','-b','nrf7002dk/nrf5340/cpuapp','-d',$taskBuild,$taskApp)
if ($Pristine) { $taskArgs += @('--pristine','always') }
$taskArgs += @('--',('-DEXTRA_DTC_OVERLAY_FILE=' + (Join-Path $taskApp 'boards\jolteon_uart.overlay').Replace('\','/')))
Write-Host 'Compiling WT02E40E firmware for Jolteon (UART RX P1.05)...'
Push-Location $taskSdk
try {
    & $taskPython @taskArgs
    if ($LASTEXITCODE -ne 0) { throw "Firmware build failed: $LASTEXITCODE" }
    Write-Host "Build complete: $taskBuild"
} finally { Pop-Location }
