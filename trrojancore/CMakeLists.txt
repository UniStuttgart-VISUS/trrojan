# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

include_directories("${IncludeDirectory}")

set(TrrojanCoreIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)

add_library(trrojancore STATIC ${HeaderFiles} ${SourceFiles})
