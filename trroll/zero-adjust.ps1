Param(
    [string] $Binary = 'C:\Users\mueller\source\repos\power-overwhelming\_build\rtx_config\Release\rtx_config.exe',
    [string] $Configuration,
    [int] $Waveforms = 8,
    [string] $ParallelPort = 'LPT3',
    [switch] $NoWait)

if (-not $Configuration) {
    $Configuration = Split-Path -parent $MyInvocation.MyCommand.Definition
    $Configuration = Join-Path $Configuration 'power2-rtx-config.json'
}

$args = '--zero-adjust', "`"$Configuration`"", '--waveforms', $Waveforms, '--apply'
if ($NoWait) {
    $args += '--imfeelinglucky'
}

Write-Host "`"$Binary`" $($args -join ' ')"
Start-Process -FilePath $Binary -NoNewWindow -Wait -ArgumentList $args

Write-Host "Patching parallel port to `"$ParallelPort`""
$content = gc $Configuration -Raw | ConvertFrom-Json
if ($content -and $content.trigger -and $content.trigger.external_trigger) {
    $content.trigger.external_trigger = $ParallelPort
}
$content | ConvertTo-Json -Depth 4 | Out-File $Configuration -Encoding UTF8 -Force
