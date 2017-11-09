# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

find_package(OpenCL)
find_package(OpenGL)

set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${IncludeDirectory}/*.hpp" "${SourceDirectory}/*.h" "${IncludeDirectory}/*.inl" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")
#file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.def")

include_directories("${IncludeDirectory}")
include_directories(${TrrojanCoreIncludeDir})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJANCL_EXPORTS -DTRROJANCORE_WITH_SPELLING_ERRORS")

#set_source_files_properties(plugin.def PROPERTIES HEADER_FILE_ONLY TRUE)

if (${OpenCL_FOUND} AND ${OPENGL_FOUND})
    add_library(trrojancl SHARED ${HeaderFiles} ${SourceFiles})

    include_directories("${OpenCL_INCLUDE_DIR}")

    target_link_libraries(trrojancl LINK_PUBLIC trrojancore)
    target_link_libraries(trrojancl LINK_PUBLIC ${OpenCL_LIBRARY})
    target_link_libraries(trrojancl LINK_PUBLIC ${OPENGL_gl_LIBRARY})
endif (${OpenCL_FOUND} AND ${OPENGL_FOUND})
