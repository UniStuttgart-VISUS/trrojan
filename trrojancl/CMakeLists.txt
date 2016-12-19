# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")
#file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.def")

include_directories("${IncludeDirectory}")
include_directories(${TrrojanCoreIncludeDir})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJANCL_EXPORTS")

#set_source_files_properties(plugin.def PROPERTIES HEADER_FILE_ONLY TRUE)

#add_library(trrojancl MODULE ${HeaderFiles} ${SourceFiles})
add_library(trrojancl SHARED ${HeaderFiles} ${SourceFiles})

target_link_libraries(trrojancl LINK_PUBLIC trrojancore)
