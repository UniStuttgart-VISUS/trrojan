# CMakeLists.txt
# Copyright (C) 2017 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")
file(GLOB_RECURSE ShaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.hlsli" "${SourceDirectory}/*.hlsl")

include_directories("${IncludeDirectory}")
include_directories("${TrrojanCoreIncludeDir}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAND3D11_EXPORTS")

if(WIN32)
	add_library(trrojand3d11 MODULE ${HeaderFiles} ${SourceFiles} ${ShaderFiles})
	target_link_libraries(trrojand3d11 trrojancore)
	target_link_libraries(trrojand3d11 d3d11 dxgi dxguid)
endif(WIN32)
