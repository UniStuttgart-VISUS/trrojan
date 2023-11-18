# ThirdParty.cmake
# Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

include(ExternalProject)
include(FetchContent)

set(FETCHCONTENT_QUIET ON)
mark_as_advanced(FORCE
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_QUIET
    FETCHCONTENT_UPDATES_DISCONNECTED)


# Chakra Core
if (NOT WIN32)
    # On Linux, we can use Cmake ... I think.
    FetchContent_Declare(ChakraCore
        URL "https://github.com/chakra-core/ChakraCore/archive/refs/tags/v1.11.24.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    FetchContent_MakeAvailable(ChakraCore)
    mark_as_advanced(FORCE
        FETCHCONTENT_SOURCE_DIR_CHAKRACORE
        FETCHCONTENT_UPDATES_DISCONNECTED_CHAKRACORE)

elseif (NOT TRROJAN_FOR_UWP)
    set(CHAKRA_CONFIGURATION "Release")

    # On Windows, we need to determine whether we are cross-compiling or
    # building for the native platform.
    if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(CHAKRA_PLATFORM "x86")
    else ()
        set (CHAKRA_PLATFORM "x64")
    endif ()

    ExternalProject_Add(ChakraCore
        URL "https://github.com/chakra-core/ChakraCore/archive/refs/tags/v1.11.24.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        PREFIX _deps/ChakraCore
        CMAKE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND msbuild /m /p:Platform=${CHAKRA_PLATFORM} /p:Configuration=${CHAKRA_CONFIGURATION} /p:RuntimeLib=static_library "Build/Chakra.Core.sln"
        INSTALL_COMMAND ""  
        BUILD_IN_SOURCE ON)

    ExternalProject_Get_Property(ChakraCore SOURCE_DIR)
    set(ChakraCore_SOURCE_DIR ${SOURCE_DIR})
    set(ChakraCore_BINARY_DIR "${ChakraCore_SOURCE_DIR}/Build/VcBuild/bin/${CHAKRA_PLATFORM}_${CHAKRA_CONFIGURATION}")
endif ()


# datraw
FetchContent_Declare(datraw
    URL "https://github.com/UniStuttgart-VISUS/datraw/archive/refs/tags/v1.0.8.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(datraw)
add_library(datraw INTERFACE IMPORTED GLOBAL)
set_target_properties(datraw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${datraw_SOURCE_DIR}/datraw")
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_DATRAW
    FETCHCONTENT_UPDATES_DISCONNECTED_DATRAW)


# DirectStorage
set(TRROJAN_DSTORAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/microsoft.direct3d.directstorage.1.2.1/native" CACHE STRING "Location of the DirectStorage APIs.")
add_library(dstorage SHARED IMPORTED GLOBAL)
set_target_properties(dstorage PROPERTIES
    IMPORTED_LOCATION "${TRROJAN_DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/dstorage.dll"
    IMPORTED_IMPLIB "${TRROJAN_DSTORAGE_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/dstorage.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${TRROJAN_DSTORAGE_DIR}/Include"
    INTERFACE_COMPILE_DEFINITIONS TRROJAN_WITH_DSTORAGE
    INTERFACE_LINK_LIBRARIES dstorage)
file(GLOB_RECURSE TRROJAN_DSTORAGE_BINARIES "${TRROJAN_DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/*.dll")


# glm
FetchContent_Declare(glm
    URL "https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(glm)
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_GLM
    FETCHCONTENT_UPDATES_DISCONNECTED_GLM)


# mmpld
FetchContent_Declare(mmpld
    URL "https://github.com/UniStuttgart-VISUS/mmpld/archive/refs/tags/v1.16.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(mmpld)
add_library(mmpld INTERFACE IMPORTED GLOBAL)
set_target_properties(mmpld PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${mmpld_SOURCE_DIR}/mmpld")
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_MMPLD
    FETCHCONTENT_UPDATES_DISCONNECTED_MMPLD)


# Power Overwhelming
if (TRROJAN_WITH_POWER_OVERWHELMING)
    FetchContent_Declare(power_overwhelming
        URL "https://github.com/UniStuttgart-VISUS/power-overwhelming/archive/refs/tags/v1.8.0.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    option(PWROWG_BuildDemo "" OFF)
    option(PWROWG_BuildDriver "" OFF)
    option(PWROWG_BuildDumpSensors "" OFF)
    option(PWROWG_BuildStablePower "" OFF)
    option(PWROWG_BuildTests "" OFF)
    option(PWROWG_BuildWeb "" OFF)
    FetchContent_MakeAvailable(power_overwhelming)
    mark_as_advanced(FORCE
        FETCHCONTENT_SOURCE_DIR_POWER_OVERWHELMING
        FETCHCONTENT_UPDATES_DISCONNECTED_POWER_OVERWHELMING
        PWROWG_BuildDemo
        PWROWG_BuildDriver
        PWROWG_BuildDumpSensors
        PWROWG_BuildStablePower
        PWROWG_BuildTests
        PWROWG_BuildWeb
        PWROWG_WithAdl
        PWROWG_WithNvml
        PWROWG_WithTimeSynchronisation
        PWROWG_WithVisa)
endif ()


# spdlog
FetchContent_Declare(spdlog
    URL "https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
option(SPDLOG_INSTALL "" ON)
option(SPDLOG_BUILD_TESTING "" OFF)
option(SPDLOG_BUILD_EXAMPLES "" OFF)
FetchContent_MakeAvailable(spdlog)
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_SPDLOG
    FETCHCONTENT_UPDATES_DISCONNECTED_SPDLOG
    SPDLOG_BUILD_TESTING
    SPDLOG_BUILD_EXAMPLES)


# We need to know whether we have OpenCL to enable the project
find_package(OpenCL)
find_package(OpenGL)


# Stuff required for CImg.
find_package(JPEG)
find_package(TIFF)
find_package(ZLIB)
find_package(PNG)
find_package(OpenMP)
