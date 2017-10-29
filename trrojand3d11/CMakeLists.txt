# CMakeLists.txt
# Copyright (C) 2017 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

# Glob shaders
file(GLOB_RECURSE VertexShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*VertexShader.hlsl")
file(GLOB_RECURSE GeometryShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*GeometryShader.hlsl")
file(GLOB_RECURSE PixelShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*PixelShader.hlsl")
file(GLOB_RECURSE ShaderHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.hlsli" "${SourceDirectory}/*.hlsli")
set(ShaderFiles ${VertexShaderFiles} ${GeometryShaderFiles} ${PixelShaderFiles})

# Configure fxc
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_ENTRYPOINT Main)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_FLAGS "/Fh \"$(OutDir)%(Filename).h\" /Vn \"%(Filename)Bytes\"")

# Configure shader types
set_property(SOURCE ${VertexShaderFiles} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${GeometryShaderFiles} PROPERTY VS_SHADER_TYPE Geometry)
set_property(SOURCE ${PixelShaderFiles} PROPERTY VS_SHADER_TYPE Pixel)

# Add hlsli files such that we have them in the solution.
file(GLOB_RECURSE ShaderHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.hlsli" "${SourceDirectory}/*.hlsli")
set(ShaderFiles ${ShaderHeaderFiles} ${ShaderFiles})


include_directories("${IncludeDirectory}")
include_directories("${TrrojanCoreIncludeDir}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAND3D11_EXPORTS")

if(WIN32)
	add_library(trrojand3d11 MODULE ${HeaderFiles} ${SourceFiles} ${ShaderFiles})
	target_link_libraries(trrojand3d11 trrojancore)
	target_link_libraries(trrojand3d11 d3d11 dxgi dxguid)
endif(WIN32)
