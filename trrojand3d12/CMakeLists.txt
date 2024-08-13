# CMakeLists.txt
# Copyright (C) 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart.

project(trrojand3d12)


# Glob and add sources and resources
set(IncludeDirectory "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SourceDirectory "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE PublicHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${IncludeDirectory}/*.inl")
file(GLOB_RECURSE PrivateHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.h" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")
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
set(ShaderOutputDirectory "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_ENTRYPOINT Main)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_MODEL 6.0)
#set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_FLAGS /Zi)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_ENABLE_DEBUG On)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_FLAGS /Qembed_debug)
set_property(SOURCE ${ShaderFiles} PROPERTY VS_SHADER_OBJECT_FILE_NAME "${ShaderOutputDirectory}/d3d12/%(Filename).cso")

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


# Define the library target
add_library(${PROJECT_NAME} SHARED ${PublicHeaderFiles} ${PrivateHeaderFiles} ${SourceFiles} ${ShaderFiles} ${ResourceFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE TRROJAND3D12_EXPORTS MMPLD_WITH_DIRECT3D)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${IncludeDirectory}>
    PRIVATE
        $<BUILD_INTERFACE:${SourceDirectory}>
        $<BUILD_INTERFACE:${ShaderOutputDirectory}>)
target_link_libraries(${PROJECT_NAME} PRIVATE trrojancore)
target_link_libraries(${PROJECT_NAME} PRIVATE d3d12 dxgi dxguid d2d1 dwrite)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

if (TRROJAN_DEBUG_OVERLAY)
    # Enable debug overlay on request.
    target_compile_definitions(${PROJECT_NAME} PRIVATE CREATE_D2D_OVERLAY)
endif ()

if (TRROJAN_WITH_DSTORAGE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dstorage)
#    foreach (TRROJAN_DSTORAGE_BINARY ${TRROJAN_DSTORAGE_BINARIES})
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TRROJAN_DSTORAGE_BINARY}" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
#    endforeach ()
endif ()


# Group all HLSL into a new filter (cf. https://stackoverflow.com/questions/33808087/cmake-how-to-create-visual-studio-filters).
source_group("Shader Files" FILES ${ShaderFiles})

# Group resource scripts into a new filter.
source_group("Resource Files" FILES ${ResourceFiles})


# Installation
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${IncludeDirectory}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
