[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]$OutputPath
)

$osInfo = Get-CimInstance Win32_OperatingSystem
$compiler = ((cl /? 2>&1 | Out-String) -split '\r')[0] -replace '^cl\s:\s',''
$jsonObject = [ordered]@{
    OS = $osInfo.Caption
    KernelVersion = $osInfo.Version
    Architecture = $env:PROCESSOR_ARCHITECTURE
    Compiler = $compiler
} | ConvertTo-Json
Set-Content -Path $OutputPath -Value $jsonObject
