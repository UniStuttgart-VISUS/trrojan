# CMakeLists.txt
# Copyright (C) 2022 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

# Glob and add resoruce scripts
file(GLOB_RECURSE ResourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.rc")

# Glob shaders
file(GLOB_RECURSE VertexShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*VertexShader*.hlsl")
file(GLOB_RECURSE HullShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*HullShader*.hlsl")
file(GLOB_RECURSE DomainShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*DomainShader*.hlsl")
file(GLOB_RECURSE GeometryShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*GeometryShader*.hlsl")
file(GLOB_RECURSE PixelShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*PixelShader*.hlsl")
file(GLOB_RECURSE ComputeShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*ComputeShader*.hlsl")
file(GLOB_RECURSE ShaderHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.hlsli" "${SourceDirectory}/*.hlsli")
set(ShaderFiles ${VertexShaderFiles} ${DomainShaderFiles} ${HullShaderFiles} ${GeometryShaderFiles} ${PixelShaderFiles} ${ComputeShaderFiles})

# Configure fxc
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_ENTRYPOINT Main)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_MODEL 6.0)
#set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_FLAGS /Zi)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_ENABLE_DEBUG On)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_FLAGS /Qembed_debug)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_OBJECT_FILE_NAME "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/d3d12/%(Filename).cso")

# Configure shader types
set_property(SOURCE ${VertexShaderFiles} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${HullShaderFiles} PROPERTY VS_SHADER_TYPE Hull)
set_property(SOURCE ${DomainShaderFiles} PROPERTY VS_SHADER_TYPE Domain)
set_property(SOURCE ${GeometryShaderFiles} PROPERTY VS_SHADER_TYPE Geometry)
set_property(SOURCE ${PixelShaderFiles} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${ComputeShaderFiles} PROPERTY VS_SHADER_TYPE Compute)

# Add hlsli files such that we have them in the solution.
file(GLOB_RECURSE ShaderHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.hlsli" "${SourceDirectory}/*.hlsli")
set(ShaderFiles ${ShaderHeaderFiles} ${ShaderFiles})

include_directories("${IncludeDirectory}")
include_directories("${TrrojanCoreIncludeDir}")
if(NOT UWP)
include_directories("${TrrojanSysInfoIncludeDir}")
endif()

# Create CSOs for packing them into the resource files.
#add_dependencies(${ResourceFiles} ${ShaderFiles})
include_directories("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAND3D12_EXPORTS")

set(TrrojanD3D12IncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)

# Enable debug overlay on rquest.
#if (TRROJAN_DEBUG_OVERLAY)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCREATE_D2D_OVERLAY")
#endif (TRROJAN_DEBUG_OVERLAY)


if (WIN32)
    if(UWP)
        add_library(trrojand3d12 SHARED ${HeaderFiles} ${SourceFiles} ${ShaderFiles} ${ResourceFiles})
    else()
        add_library(trrojand3d12 MODULE ${HeaderFiles} ${SourceFiles} ${ShaderFiles} ${ResourceFiles})
    endif()
    target_link_libraries(trrojand3d12 trrojancore)
    target_link_libraries(trrojand3d12 d3d12 dxgi dxguid d2d1 dwrite)

    ## Reference DirectStorage (cf. https://cmake.org/cmake/help/latest/prop_tgt/VS_PACKAGE_REFERENCES.html)
    #set_property(TARGET ${TARGET_NAME} PROPERTY VS_PACKAGE_REFERENCES "Microsoft.Direct3D.DirectStorage")

    # Group all HLSL into a new filter (cf. https://stackoverflow.com/questions/33808087/cmake-how-to-create-visual-studio-filters).
    source_group("Shader Files" FILES ${ShaderFiles})

    # Group resource scripts into a new filter.
    source_group("Resource Files" FILES ${ResourceFiles})
endif (WIN32)
