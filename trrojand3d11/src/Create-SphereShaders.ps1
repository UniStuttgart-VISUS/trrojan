[CmdletBinding()]
param([Parameter(Mandatory = $true, ValueFromPipeline = $true)] [string] $OutPath)

begin {
    # Properties of the rendering techniques.
    $SPHERE_TECHNIQUE_USE_GEO = ([uint64] 1) -shl 31
    $SPHERE_TECHNIQUE_USE_TESS = ([uint64] 1) -shl 30
    $SPHERE_TECHNIQUE_USE_SRV =  ([uint64] 1) -shl 29
    
    # Rendering techniques.    
    $__SPHERE_TECH_BASE = ([uint64] 1) -shl 63
    $techniques = @{}
    $techniques[($__SPHERE_TECH_BASE -shr 0) -bor $SPHERE_TECHNIQUE_USE_SRV] = "QUAD_INST"
    $techniques[($__SPHERE_TECH_BASE -shr 1) -bor $SPHERE_TECHNIQUE_USE_SRV] = "POLY_INST"
    $techniques[($__SPHERE_TECH_BASE -shr 2) -bor $SPHERE_TECHNIQUE_USE_TESS] = "QUAD_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 3) -bor $SPHERE_TECHNIQUE_USE_TESS] = "POLY_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 4) -bor $SPHERE_TECHNIQUE_USE_TESS] = "ADAPT_POLY_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 5) -bor $SPHERE_TECHNIQUE_USE_GEO] = "SPTA"
    $techniques[($__SPHERE_TECH_BASE -shr 6) -bor $SPHERE_TECHNIQUE_USE_GEO] = "GEO_QUAD"
    $techniques[($__SPHERE_TECH_BASE -shr 7) -bor $SPHERE_TECHNIQUE_USE_GEO] = "GEO_POLY"
    $techniques[($__SPHERE_TECH_BASE -shr 8) -bor $SPHERE_TECHNIQUE_USE_TESS] = "SPHERE_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 9) -bor $SPHERE_TECHNIQUE_USE_TESS] = "ADAPT_SPHERE_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 10) -bor $SPHERE_TECHNIQUE_USE_TESS] = "HEMISPHERE_TESS"
    $techniques[($__SPHERE_TECH_BASE -shr 11) -bor $SPHERE_TECHNIQUE_USE_TESS] = "ADAPT_HEMISPHERE_TESS"

    # Properties of the input data.
    $SPHERE_INPUT_PV_COLOUR = ([uint64] 1) -shl 0
    $SPHERE_INPUT_PV_RADIUS = ([uint64] 1) -shl 1
    $SPHERE_INPUT_PV_INTENSITY = ([uint64] 1) -shl 2
    $SPHERE_INPUT_PP_INTENSITY = ([uint64] 1) -shl 15
    $SPHERE_INPUT_FLT_COLOUR = ([uint64] 1) -shl 16


    function Create-Shader([string] $file, [string] $core, [uint64] $technique, [uint64] $features) {
        $featureCode = ("{0:X16}" -f ($technique -bor $features))

        $lines = @()
        $lines += "// This file was auto-generated using Create-SphereShaders.ps1 on $(Get-Date)"

        $lines += "#define $($techniques[$technique]) (1)"

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

        $lines += "#include `"$core`""

        $lines | Out-File -FilePath "$file$featureCode.hlsl" -Encoding ascii 
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

            $techniques.Keys | %{
                $technique = $_
                
                $cntFlt = 0
                if (($technique -band $SPHERE_TECHNIQUE_USE_SRV) -and ($pvColour -ne 0)) {
                    # Technique uses structured resource view and format includes colour,
                    # so we need to include in-shader RGB8 to float conversion and host code
                    # conversion.
                    $cntFlt = 1
                }

                $cntXfer = 0
                if ($pvIntensity -ne 0) {
                    # Technique has per-vertex intensity, so we need to test VS texture lookup
                    # and PS texture lookup.
                    $cntXfer = 1
                }
                
                0..$cntFlt | %{
                    $flt = $_ * $SPHERE_INPUT_FLT_COLOUR

                    0..$cntXfer | %{
                        $xfer = ($_ * $SPHERE_INPUT_PV_INTENSITY) -bor ((1 - $_) * $SPHERE_INPUT_PP_INTENSITY)
                        $features = ($pvRadius -bor $pvColour -bor $flt -bor $xfer)
                        
                        Create-Shader (Join-Path $OutPath "SphereVertexShader") "SphereVertexShaderCore.hlsli" $technique $features

                        if ($technique -band $SPHERE_TECHNIQUE_USE_GEO) {
                            Create-Shader (Join-Path $OutPath "SphereGeometryShader") "SphereGeometryShaderCore.hlsli" $technique $features
                        }

                        if ($technique -band $SPHERE_TECHNIQUE_USE_TESS) {
                            Create-Shader (Join-Path $OutPath "SphereHullShader") "SphereHullShaderCore.hlsli" $technique $features
                            Create-Shader (Join-Path $OutPath "SphereDomainShader") "SphereDomainShaderCore.hlsli" $technique $features
                        }

                        Create-Shader (Join-Path $OutPath "SpherePixelShader") "SpherePixelShaderCore.hlsli" $technique $features
                    }
                    
                }
            }
          
        }
    }
}

end { }

