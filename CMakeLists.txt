# CMakeLists.txt
# Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.24.0)
include(CMakeDependentOption)


project(TRRojan)

# Detect UWP
if (${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
    set(TRROJAN_FOR_UWP TRUE)
else ()
    set(TRROJAN_FOR_UWP FALSE)
endif ()


# Customise TRRojan
cmake_dependent_option(TRROJAN_WITH_CHAKRA "Enable JavaScript support." OFF "NOT TRROJAN_FOR_UWP" OFF)
cmake_dependent_option(TRROJAN_WITH_DSTORAGE "Enable support for DirectStorage in Direct3D 12 plugin." ON WIN32 OFF)
cmake_dependent_option(TRROJAN_FORCE_NO_D3D_DEBUG "Force the debug layer to be disabled." OFF WIN32 OFF)
cmake_dependent_option(TRROJAN_WITH_POWER_OVERWHELMING "Enable power_overwhelming for measuring GPU power consumption." ON "NOT TRROJAN_FOR_UWP" OFF)
option(TRROJAN_DEBUG_OVERLAY "Enable overlay in debug view." OFF)
set(TRROJAN_UWP_PLATFORM_VERSION "10.0.19041.0" CACHE STRING "Specifies the minimum target platform version for UWP.")


# Third-party software
include("ThirdParty.cmake")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION 19043)

add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

# Global compiler configuration for all of TRRojan.
if (WIN32)
    add_compile_options(/wd4251)	# We cannot fix that - TRRojan just needs to be compiled in a single step.
endif (WIN32)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-DUNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
endif ()

if (PNG_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAN_WITH_CIMG -Dcimg_display=0")
endif ()

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (TRROJAN_WITH_DSTORAGE)
    # Include our downloaded version of DirectStorage, because VS_PACKAGE_REFERENCES
    # is not working for CMAKE C++ projects.
    include_directories("${DSTORAGE_DIR}/Include/DirectStorage")
    add_definitions(-DTRROJAN_WITH_DSTORAGE)
    link_directories("${DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}")
endif ()

if (TRROJAN_FOR_UWP)
    add_compile_definitions(_UWP)
    add_compile_definitions(__cplusplus_winrt)
    add_compile_definitions(MMPLD_WITHOUT_WIN32_IO)
    add_compile_definitions(TRROJAN_FOR_UWP)
endif ()

if (TRROJAN_FORCE_NO_D3D_DEBUG)
    add_compile_definitions(TRROJAN_FORCE_NO_D3D_DEBUG)
endif()


# Build the system information library.
if (NOT TRROJAN_FOR_UWP)
    add_subdirectory(trrojansnfo)
endif()

# Build the core library
add_subdirectory(trrojancore)

# Build the OpenCL plugin if we can
if (OpenCL_FOUND AND OPENGL_FOUND AND NOT TRROJAN_FOR_UWP)
    set(TRROJAN_WITH_OPENCL TRUE)
    add_subdirectory(trrojancl)
    set(TRROJAN_PLUGINS ${TRROJAN_PLUGINS} trrojancl)
endif ()

# Build the RAM stream plugin
add_subdirectory(trrojanstream)
set(TRROJAN_PLUGINS ${TRROJAN_PLUGINS} trrojanstream)

# Build the D3D plugins.
if (WIN32)
    add_subdirectory(trrojand3d11)
    set(TRROJAN_PLUGINS ${TRROJAN_PLUGINS} trrojand3d11)
    add_subdirectory(trrojand3d12)
    set(TRROJAN_PLUGINS ${TRROJAN_PLUGINS} trrojand3d12)
endif ()

# Build the executable
add_subdirectory(trrojan)
