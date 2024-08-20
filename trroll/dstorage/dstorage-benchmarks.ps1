$trrollroot = "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage"
$trrojanexe = "C:\Users\mueller\source\repos\TRRojan\_twelve\trrojan\Release\trrojan.exe"

$scripts = @("C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\stream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\streamds.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\dstorage.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\static.trroll")
$scripts = @("C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\d3d11static.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\d3d12static.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\stream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\streamds.trroll")
$scripts = @("C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\ramstream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\dstorage.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\streamds.trroll")
$scripts = @("C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\d3d11static.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\d3d12static.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\dstoragenaive.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\dstoragebatches.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\dstoragegdeflate.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\ramstream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\stream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\streamds.trroll")
$scripts = @("C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\stream.trroll", "C:\Users\mueller\source\repos\TRRojan\trroll\dstorage\streamds.trroll")
$scripts = @((Join-Path $trrollroot "d3d11static.trroll"), (Join-Path $trrollroot "d3d12static.trroll"), (Join-Path $trrollroot "dstoragebatches.trroll"), (Join-Path $trrollroot "dstoragegdeflate.trroll"), (Join-Path $trrollroot "dstoragenaive.trroll"), (Join-Path $trrollroot "ramstream.trroll"), (Join-Path $trrollroot "stream.trroll"), (Join-Path $trrollroot "streamds.trroll"))
$scripts = @((Join-Path $trrollroot "stream.trroll"), (Join-Path $trrollroot "streamds.trroll"))
$scripts = @((Join-Path $trrollroot "dstoragesplitbatches.trroll"))


$scripts | %{
    $script = $_
    $outdir = Split-Path -Parent $trrojanexe
    $output = Join-Path $outdir ([System.IO.Path]::ChangeExtension((Split-Path $script -Leaf), ".xlsx"))
    $log = Join-Path $outdir ([System.IO.Path]::ChangeExtension((Split-Path $script -Leaf), ".log"))
    Start-Process -FilePath $trrojanexe -ArgumentList "--trroll", $script, "--output", $output, "--log", $log, "--visible", "--unique-devices" -Wait
}
