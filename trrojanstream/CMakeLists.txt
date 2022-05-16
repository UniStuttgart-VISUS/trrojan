# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

find_package (Threads)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

include_directories("${IncludeDirectory}")
include_directories("${TrrojanCoreIncludeDir}")
#include_directories("${TrrojanSysInfoIncludeDir}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJANSTREAM_EXPORTS")

add_library(trrojanstream MODULE ${HeaderFiles} ${SourceFiles})

# HACK:
#set(TrrojanStreamIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)
#add_library(trrojanstream SHARED ${HeaderFiles} ${SourceFiles})

target_link_libraries(trrojanstream LINK_PUBLIC trrojancore ${CMAKE_THREAD_LIBS_INIT})
