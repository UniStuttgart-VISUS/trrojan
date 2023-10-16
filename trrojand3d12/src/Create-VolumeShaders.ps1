[CmdletBinding()]
param([Parameter(Mandatory = $true, ValueFromPipeline = $true)] [string] $OutPath,
    [string] $IncludeFile = "volume_techniques.h",
    [string] $ResourceFile = "volume_techniques.rc",
    [uint16] $ResourceStart = 512,
    [string] $ResourcePath = "d3d12",
    [string] $ResourceType = "SHADER")

# The lines of the resource file to be generated.
$resources = @()
$resources += "// This file was auto-generated using $($MyInvocation.Line) on $(Get-Date)"

# The lines of the include file to be generated.
$includes = @()
$includes += "// This file was auto-generated using $($MyInvocation.Line) on $(Get-Date)"

# Add the shaders
$cnt = 0
Get-ChildItem -Filter "*Volume*Shader.hlsl" | ForEach-Object {
    $variable = ($_.BaseName -csplit "(?<!^)(?=[A-Z])" -join "_").ToUpperInvariant()
    $objectFile = Join-Path $ResourcePath "$($_.BaseName).cso"
    $resource = $ResourceStart + $cnt
    $includes += "#define $variable ($resource)"
    $includes += "#define $($variable)_PATH `"$objectFile`""
    $resources += "$resource $ResourceType $objectFile"
    ++$cnt
}

$includes | Out-File -FilePath (Join-Path $OutPath $IncludeFile) -Encoding ascii
$resources | Out-File -FilePath (Join-Path $OutPath $ResourceFile) -Encoding ascii

Write-Verbose "$cnt have been generated."
