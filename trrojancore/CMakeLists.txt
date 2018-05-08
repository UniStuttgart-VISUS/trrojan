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
include_directories("${TrrojanSysInfoIncludeDir}")

add_definitions("-DTRROJANCORE_EXPORTS")

set(TrrojanCoreIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)

add_library(trrojancore SHARED ${HeaderFiles} ${SourceFiles})

# Disable Chakra core if we cannot build it.
if (NOT EXISTS ${CHAKRA_DIR})
    MESSAGE(WARNING "Chakra Core is being disabled because the source directory does not exist.")
    set(TRROJAN_WITH_CHAKRA OFF)
endif(NOT EXISTS ${CHAKRA_DIR})
#if (NOT WIN32)
#    set(TRROJAN_WITH_CHAKRA OFF)
#endif()

# Build Chakra core if possible
if (TRROJAN_WITH_CHAKRA)
    set(CHAKRA_CONFIGURATION "Release")

    add_definitions("-DWITH_CHAKRA")
    include_directories("${CHAKRA_DIR}/lib/Jsrt")

    # On Windows, we need to determine whether we are cross-compiling or
    # building for the native platform.
    if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(CHAKRA_PLATFORM "x86")
    else ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set (CHAKRA_PLATFORM "x64")
    endif ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")

    if (WIN32)
        set (CHAKRA_BIN_DIR "${CHAKRA_DIR}/Build/VcBuild/bin/${CHAKRA_PLATFORM}_${CHAKRA_CONFIGURATION}")
        set (CHAKRA_BIN "${CHAKRA_BIN_DIR}/chakracore.dll")
        set (CHAKRA_LIB "${CHAKRA_BIN_DIR}/chakracore.lib")
        add_custom_command(TARGET trrojancore
                           PRE_BUILD
                           COMMAND msbuild /m /p:Platform=${CHAKRA_PLATFORM} /p:Configuration=${CHAKRA_CONFIGURATION} /p:RuntimeLib=static_library "${CHAKRA_DIR}/Build/Chakra.Core.sln"
                           COMMENT "Building Chakra Core ...")
        add_custom_command(TARGET trrojancore
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CHAKRA_BIN} $<TARGET_FILE_DIR:trrojancore>)
        target_link_libraries(trrojancore ${CHAKRA_LIB})

    else (WIN32)
        set (CHAKRA_BIN_DIR "${CHAKRA_DIR}/out/${CHAKRA_CONFIGURATION}")
        set (CHAKRA_LIB "${CHAKRA_BIN_DIR}/libChakraCore.so")

        add_custom_command(OUTPUT ${CHAKRA_LIB}
                           COMMAND ${CHAKRA_DIR}/build.sh --libs-only --extra-defines CMAKE_POSITION_INDEPENDENT_CODE=ON
                           COMMENT "Building Chakra Core ...")
        add_custom_target(chakra DEPENDS ${CHAKRA_LIB})
        add_dependencies(trrojancore chakra)

        add_library(ChakraCore STATIC IMPORTED)
        set_property(TARGET ChakraCore PROPERTY IMPORTED_LOCATION ${CHAKRA_LIB})

        target_link_libraries(trrojancore ChakraCore)
    endif (WIN32)
endif (TRROJAN_WITH_CHAKRA)

# Configure the linker.
target_link_libraries(trrojancore trrojansnfo)

if (${CMAKE_COMPILER_IS_GNUCXX})
    target_link_libraries(trrojancore dl)
endif (${CMAKE_COMPILER_IS_GNUCXX})
