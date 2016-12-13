# CMakeLists.txt
# Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"  "*.h")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"  "*.cpp")

include_directories(${TrrojanCoreIncludeDir})

add_executable(trrojan ${SourceFiles})

target_link_libraries(trrojan LINK_PUBLIC trrojancore)
