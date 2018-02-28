# CMakeLists.txt
# Copyright (C) 2018 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)


set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

# Glob and add resoruce scripts
file(GLOB_RECURSE ResourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.rc")

include_directories("${IncludeDirectory}")

add_definitions("-DTRROJANSNFO_EXPORTS")

set(TrrojanSysInfoIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)

add_library(trrojansnfo SHARED ${HeaderFiles} ${SourceFiles} ${ResourceFiles})

if (WIN32)
target_link_libraries(trrojansnfo d3d11 dxgi dxguid)
endif (WIN32)

# Group resource scripts into a new filter.
source_group("Resource Files" FILES ${ResourceFiles})

