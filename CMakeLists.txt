# CMakeLists.txt
# Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.15.0)
include(CMakeDependentOption)
include(GNUInstallDirs)


project(TRRojan)

# Detect UWP
set(TRROJAN_FOR_UWP ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")


# Customise TRRojan
cmake_dependent_option(TRROJAN_WITH_CHAKRA "Enable JavaScript support." OFF NOT ${TRROJAN_FOR_UWP} OFF)
cmake_dependent_option(TRROJAN_WITH_DSTORAGE "Enable support for DirectStorage in Direct3D 12 plugin." ON WIN32 OFF)
cmake_dependent_option(TRROJAN_WITH_POWER_OVERWHELMING "Enable power_overwhelming for measuring GPU power consumption." ON NOT ${TRROJAN_FOR_UWP} OFF)
option(TRROJAN_DEBUG_OVERLAY "Enable overlay in debug view." OFF)


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

if (${MSVC})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
endif ()

if (${CMAKE_COMPILER_IS_GNUCXX})
    add_definitions(-DUNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
endif ()

if (${PNG_FOUND})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAN_WITH_CIMG -Dcimg_display=0")
endif ()

if (${OPENMP_FOUND})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (${TRROJAN_WITH_DSTORAGE})
    # Include our downloaded version of DirectStorage, because VS_PACKAGE_REFERENCES
    # is not working for CMAKE C++ projects.
    include_directories("${DSTORAGE_DIR}/Include/DirectStorage")
    add_definitions(-DTRROJAN_WITH_DSTORAGE)
    link_directories("${DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}")
endif ()


if (NOT ${TRROJAN_FOR_UWP})
    # Build the system information library.
    add_subdirectory(trrojansnfo)
endif()

# Build the core library
add_subdirectory(trrojancore)

# Build the OpenCL plugin
set(WITH_OPENCL ${OpenCL_FOUND} AND ${OPENGL_FOUND} AND NOT ${TRROJAN_FOR_UWP})
if (${WITH_OPENCL})
    add_subdirectory(trrojancl)
endif ()

# Build the RAM stream plugin
add_subdirectory(trrojanstream)

# Build the D3D plugins.
if (WIN32)
    add_subdirectory(trrojand3d11)
    add_subdirectory(trrojand3d12)
endif ()

# Build the executable
add_subdirectory(trrojan)
