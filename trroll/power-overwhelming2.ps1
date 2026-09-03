# TRRojan binary to execute
$bin = 'C:\Users\mueller\source\repos\trrojan\'
$bin = 'T:\Programmcode\trrojan-github\_build\trrojan\Debug\trrojan.exe'
# Location where the $trroll scripts are located.
$scripts = 'C:\Users\mueller\source\repos\trrojan\trroll'
$scripts = 'T:\Programmcode\trrojan-github\trroll'
# Location where the output files are written to.
$out = 'C:\Users\mueller\Documents\pwrowg2'
$out = 'T:\Programmcode\trrojan-github\'
# RTx oscilloscope configuration.
$rtx = Join-Path $scripts '\power2-rtx-config.json'
$rtx = $null
# List of TRRoll scripts in $scripts to execute.
$troll = ('power2-rayspheres.trroll', 'power2-tessspheres.trroll')

if (-not (Test-Path -PathType Container -Path $out)) {
    throw "Output directory `"$out`" does not exist."
}

$troll | %{
    $b = gi $bin
    Write-Host "Binary `"$b`" was built at $($b.LastWriteTime)"

    $t = Join-Path $scripts $_
    Write-Host "Running TRRoll script `"$t`"."

    if (-not(Test-Path -PathType Leaf -Path $t)) {
        throw "TRRoll script `"$t`" does not exist."
    }

    $l = Join-Path $out ([System.IO.Path]::ChangeExtension($_, '.txt'))
    Write-Host "Log output is `"$l`"."

    $o = Join-Path $out ([System.IO.Path]::ChangeExtension("timings-$_", '.csv'))
    Write-Host "Timing output is `"$o`"."

    $p = Join-Path $out ([System.IO.Path]::ChangeExtension("power-$_", '.csv'))
    Write-Host "Power output is `"$p`"."

    $args = '--nologo', '--trroll', $t, '--log', $l, '--output', $o, '--power', $p
    if ($rtx) {
        $args += '--rtx-configuration'
        $rtx += $rtx
    }

    Write-Host "`"$bin`" $($args -join ' ')"
    Start-Process -FilePath $bin -NoNewWindow -Wait -ArgumentList $args
}
