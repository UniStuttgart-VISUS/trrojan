# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")
set(LibraryDirectory "lib")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

include_directories("${IncludeDirectory}")
include_directories("${LibraryDirectory}")

set(TrrojanCoreIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJANCORE_EXPORTS")

if (WITH_CHAKRA==ON)
endif (WITH_CHAKRA==ON)

add_library(trrojancore SHARED ${HeaderFiles} ${SourceFiles})

if (${CMAKE_COMPILER_IS_GNUCXX})
    target_link_libraries(trrojancore dl)
endif (${CMAKE_COMPILER_IS_GNUCXX})
