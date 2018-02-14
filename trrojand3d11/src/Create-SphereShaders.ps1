[CmdletBinding()]
param([Parameter(Mandatory = $true, ValueFromPipeline = $true)] [string] $OutPath,
    [string] $IncludeFile = "sphere_techniques.h")

begin {
    # Properties of the rendering techniques.
    $SPHERE_TECHNIQUE_USE_GEO = ([uint64] 1) -shl 31
    $SPHERE_TECHNIQUE_USE_TESS = ([uint64] 1) -shl 30
    $SPHERE_TECHNIQUE_USE_SRV =  ([uint64] 1) -shl 29
    $SPHERE_TECHNIQUE_USE_RAYCASTING = ([uint64] 1) -shl 28
    $SPHERE_TECHNIQUE_USE_INSTANCING = ([uint64] 1) -shl 27
    
    # Rendering techniques.    
    $__SPHERE_TECH_BASE = ([uint64] 1) -shl 63
    $global:techniques = @{}
    $global:techniques[($__SPHERE_TECH_BASE -shr 0) -bor $SPHERE_TECHNIQUE_USE_SRV -bor $SPHERE_TECHNIQUE_USE_RAYCASTING -bor $SPHERE_TECHNIQUE_USE_INSTANCING] = "QUAD_INST"
    #$global:techniques[($__SPHERE_TECH_BASE -shr 1) -bor $SPHERE_TECHNIQUE_USE_SRV -bor $SPHERE_TECHNIQUE_USE_RAYCASTING -bor $SPHERE_TECHNIQUE_USE_INSTANCING] = "POLY_INST"
    $global:techniques[($__SPHERE_TECH_BASE -shr 2) -bor $SPHERE_TECHNIQUE_USE_TESS -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "QUAD_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 3) -bor $SPHERE_TECHNIQUE_USE_TESS -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "POLY_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 4) -bor $SPHERE_TECHNIQUE_USE_TESS -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "ADAPT_POLY_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 5) -bor $SPHERE_TECHNIQUE_USE_GEO -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "STPA"
    $global:techniques[($__SPHERE_TECH_BASE -shr 6) -bor $SPHERE_TECHNIQUE_USE_GEO -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "GEO_QUAD"
    $global:techniques[($__SPHERE_TECH_BASE -shr 7) -bor $SPHERE_TECHNIQUE_USE_GEO -bor $SPHERE_TECHNIQUE_USE_RAYCASTING] = "GEO_POLY"
    $global:techniques[($__SPHERE_TECH_BASE -shr 8) -bor $SPHERE_TECHNIQUE_USE_TESS] = "SPHERE_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 9) -bor $SPHERE_TECHNIQUE_USE_TESS] = "ADAPT_SPHERE_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 10) -bor $SPHERE_TECHNIQUE_USE_TESS] = "HEMISPHERE_TESS"
    $global:techniques[($__SPHERE_TECH_BASE -shr 11) -bor $SPHERE_TECHNIQUE_USE_TESS] = "ADAPT_HEMISPHERE_TESS"
    # Note: Number of techniques must be below 32!

    # Properties of the input data.
    $SPHERE_INPUT_PV_COLOUR = ([uint64] 1) -shl 0
    $SPHERE_INPUT_PV_RADIUS = ([uint64] 1) -shl 1
    $SPHERE_INPUT_PV_INTENSITY = ([uint64] 1) -shl 2
    $SPHERE_INPUT_PP_INTENSITY = ([uint64] 1) -shl 16
    $SPHERE_INPUT_FLT_COLOUR = ([uint64] 1) -shl 17

    # Variants of the renderer
    $SPHERE_VARIANT_PV_RAY = ([uint64] 1) -shl 18
    $SPHERE_VARIANT_CONSERVATIVE_DEPTH = ([uint64] 1) -shl 19

    # List of files generated.
    $global:files = @()

    function Create-Shader([string] $fileBase, [string] $core, [uint64] $technique, [uint64] $features) {
        $featureCode = ("{0:X16}" -f ($technique -bor $features))
        $file = "$fileBase$featureCode.hlsl"
        $global:files += "$fileBase$featureCode.h"

        $lines = @()
        $lines += "// This file was auto-generated using Create-SphereShaders.ps1 on $(Get-Date)"

        $lines += "#define $($global:techniques[$technique]) (1)"


        if ($features -band $SPHERE_INPUT_PV_COLOUR) {
            $lines += '#define PER_VERTEX_COLOUR (1)'
        }
        if ($features -band $SPHERE_INPUT_FLT_COLOUR) {
            $lines += '#define FLOAT_COLOUR (1)'
        }
        if ($features -band $SPHERE_INPUT_PV_RADIUS) {
            $lines += '#define PER_VERTEX_RADIUS (1)'
        }
        if ($features -band $SPHERE_INPUT_PV_INTENSITY) {
            $lines += '#define PER_VERTEX_INTENSITY (1)'
        }
        if ($features -band $SPHERE_INPUT_PP_INTENSITY) {
            $lines += '#define PER_PIXEL_INTENSITY (1)'
        }
        if ($features -band $SPHERE_VARIANT_PV_RAY) {
            $lines += '#define PER_VERTEX_RAY (1)'
        }
        if ($technique -band $SPHERE_TECHNIQUE_USE_RAYCASTING) {
            $lines += '#define RAYCASTING (1)'
        }
        if ($technique -band $SPHERE_TECHNIQUE_USE_INSTANCING) {
            $lines += '#define INSTANCING (1)'
        }
        if ($technique -band $SPHERE_VARIANT_CONSERVATIVE_DEPTH) {
            $lines += '#define CONSERVATIVE_DEPTH (1)'
        }

        $lines += "#include `"$core`""

        $file = Join-Path $OutPath $file
        $lines | Out-File -FilePath $file -Encoding ascii 
    }

    function Create-Include([string] $file) {
        $lines = @()
        $lines += "// This file was auto-generated using Create-SphereShaders.ps1 on $(Get-Date)"

        $global:files | %{
            $lines += "#include `"$_`""
        }

        $lines += "#define SPHERE_TECHNIQUE_USE_GEO ($("0x{0:X}" -f $SPHERE_TECHNIQUE_USE_GEO))"
        $lines += "#define SPHERE_TECHNIQUE_USE_TESS ($("0x{0:X}" -f $SPHERE_TECHNIQUE_USE_TESS))"
        $lines += "#define SPHERE_TECHNIQUE_USE_SRV ($("0x{0:X}" -f $SPHERE_TECHNIQUE_USE_SRV))"
        $lines += "#define SPHERE_TECHNIQUE_USE_RAYCASTING ($("0x{0:X}" -f $SPHERE_TECHNIQUE_USE_RAYCASTING))"
        $lines += "#define SPHERE_TECHNIQUE_USE_INSTANCING ($("0x{0:X}" -f $SPHERE_TECHNIQUE_USE_INSTANCING))"
        $lines += "#define SPHERE_INPUT_PV_COLOUR ($("0x{0:X}" -f $SPHERE_INPUT_PV_COLOUR))"
        $lines += "#define SPHERE_INPUT_PV_RADIUS ($("0x{0:X}" -f $SPHERE_INPUT_PV_RADIUS))"
        $lines += "#define SPHERE_INPUT_PV_INTENSITY ($("0x{0:X}" -f $SPHERE_INPUT_PV_INTENSITY))"
        $lines += "#define SPHERE_INPUT_PP_INTENSITY ($("0x{0:X}" -f $SPHERE_INPUT_PP_INTENSITY))"
        $lines += "#define SPHERE_INPUT_FLT_COLOUR ($("0x{0:X}" -f $SPHERE_INPUT_FLT_COLOUR))"
        $lines += "#define SPHERE_VARIANT_PV_RAY ($("0x{0:X}" -f $SPHERE_VARIANT_PV_RAY))"
        $lines += "#define SPHERE_VARIANT_CONSERVATIVE_DEPTH ($("0x{0:X}" -f $SPHERE_VARIANT_CONSERVATIVE_DEPTH))"
        $global:techniques.Keys | %{
            $lines += "#define SPHERE_TECHNIQUE_$($global:techniques[$_]) ($("0x{0:X}" -f $_))"
        }

        $file = Join-Path $OutPath $file
        $lines | Out-File -FilePath $file -Encoding ascii 
    }

}

process {
    0..1 | %{
        $pvRadius = $_ * $SPHERE_INPUT_PV_RADIUS
        
        0..2 | %{
            $pvColour = ([int] ($_ / 2)) * $SPHERE_INPUT_PV_COLOUR
            $pvIntensity = ($_ % 2) * $SPHERE_INPUT_PV_INTENSITY
            #echo "RAD $pvRadius"
            #echo "COL $pvColour"
            #echo "INT $pvIntensity"

            $cntXfer = 0
            if ($pvIntensity -ne 0) {
                # Technique has per-vertex intensity, so we need to test VS
                # texture lookup and PS texture lookup.
                $cntXfer = 1
            }

            0..$cntXfer | %{
                $xfer = ($_ * $SPHERE_INPUT_PP_INTENSITY) -bor ((1 - $_) * $pvIntensity)
                                
                $global:techniques.Keys | %{
                    $technique = $_
                
                    $cntFlt = 0
                    if (($technique -band $SPHERE_TECHNIQUE_USE_SRV) -and ($pvColour -ne 0)) {
                        # Technique uses structured resource view and format
                        # includes colour, so we need to include in-shader RGB8 to 
                        # float conversion and host code conversion.
                        $cntFlt = 1
                    }

                    $cntRay = 0
                    $cntConvDepth = 0
                    if ($technique -band $SPHERE_TECHNIQUE_USE_RAYCASTING) {
                        # If the technique uses raycasting, test per-vertex and
                        # per-pixel generation of rays. Also, raycasting supports
                        # optional conservative depth.
                        $cntRay = 1
                        $cntConvDepth = 1
                    }

                    0..$cntFlt | %{
                        $flt = $_ * $SPHERE_INPUT_FLT_COLOUR


                        0..$cntRay | %{
                            $pvRay = $_ * $SPHERE_VARIANT_PV_RAY

                            0..$cntConvDepth | %{
                                $convDepth = $_ * $SPHERE_VARIANT_CONSERVATIVE_DEPTH

                                $features = ($pvRadius -bor $pvColour -bor $flt -bor $xfer -bor $pvRay -bor $convDepth)
                        
                                Create-Shader "SphereVertexShader" "SphereVertexShaderCore.hlsli" $technique $features

                                if ($technique -band $SPHERE_TECHNIQUE_USE_GEO) {
                                    Create-Shader "SphereGeometryShader" "SphereGeometryShaderCore.hlsli" $technique $features
                                }

                                if ($technique -band $SPHERE_TECHNIQUE_USE_TESS) {
                                    Create-Shader "SphereHullShader" "SphereHullShaderCore.hlsli" $technique $features
                                    Create-Shader "SphereDomainShader" "SphereDomainShaderCore.hlsli" $technique $features
                                }

                                Create-Shader "SpherePixelShader" "SpherePixelShaderCore.hlsli" $technique $features
                            }
                        }
                    }
                }
            }
        }
    }

    Write-Verbose "$($global:files.Count) shader file(s) written ..."

    Create-Include $IncludeFile
}

end { }

