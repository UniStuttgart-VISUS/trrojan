# TRRojan binary to execute
$bin = 'T:\Programmcode\trrojan-github\_build\trrojan\Debug\trrojan.exe'
$bin = 'C:\Users\mueller\source\repos\trrojan\_build\trrojan\Debug\trrojan.exe'
$bin = 'C:\Users\mueller\source\repos\trrojan\_build\trrojan\Release\trrojan.exe'
#$bin = 'C:\Users\mueller\source\repos\trrojan-old\_build\trrojan\Release\trrojan.exe'

# Location where the $trroll scripts are located.
$scripts = 'T:\Programmcode\trrojan-github\trroll'
$scripts = 'C:\Users\mueller\source\repos\trrojan\trroll'

# Location where the output files are written to.
$out = 'C:\Users\mueller\Documents\pwrowg2'
#$out = 'T:\Programmcode\trrojan-github\'

# RTx oscilloscope configuration.
$rtx = Join-Path $scripts '\power2-rtx-config.json'
#$rtx = $null

# List of TRRoll scripts in $scripts to execute.
$troll = ('power2-rayspheres.trroll', 'power2-tessspheres.trroll')
$troll = ('power2-rayspheres.trroll')
#$troll = ('power2-tessspheres.trroll')
$excluded = 'Intel(R) Graphics'

if (-not (Test-Path -PathType Container -Path $out)) {
    throw "Output directory `"$out`" does not exist."
}

$troll | %{
    $d = (Get-Date -Format "yyyyMMddHHmmss")
    $b = gi $bin
    Write-Host "Binary `"$b`" was built at $($b.LastWriteTime)"

    $t = Join-Path $scripts $_
    Write-Host "Running TRRoll script `"$t`"."

    if (-not(Test-Path -PathType Leaf -Path $t)) {
        throw "TRRoll script `"$t`" does not exist."
    }

    $l = Join-Path $out ([System.IO.Path]::ChangeExtension("$d-$_", '.log'))
    Write-Host "Log output is `"$l`"."

    $o = Join-Path $out ([System.IO.Path]::ChangeExtension("$d-timings-$_", '.csv'))
    Write-Host "Timing output is `"$o`"."

    if ($b -imatch "-old") {
        $p = Join-Path $out ([System.IO.Path]::ChangeExtension("$d-power-$_", '.csv'))
    } else {
        $p = Join-Path $out ([System.IO.Path]::ChangeExtension("$d-power-$_", '.parquet'))
    }
    Write-Host "Power output is `"$p`"."

    $s = Join-Path $out ([System.IO.Path]::ChangeExtension("$d-sensors-$_", '.json'))
    Write-Host "Sensor dump is `"$s`"."

    $args = '--nologo', '--trroll', $t, '--log', $l, '--output', $o, '--power', $p, '--dump-power-sensors', $s
    if ($rtx) {
        $args += '--rtx-configuration'
        $args += $rtx
    }
    if ($excluded) {
        $args += '--exclude-device'
        $args += $excluded
    }

    Write-Host "`"$bin`" $($args -join ' ')"
    Start-Process -FilePath $bin -NoNewWindow -Wait -ArgumentList $args
}
