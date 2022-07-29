# CMakeLists.txt
# Copyright © 2016 - 2022 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.15.0)

project(TRRojan)

find_package(JPEG)
find_package(TIFF)
find_package(ZLIB)
find_package(PNG)
find_package(OpenMP)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Configure the output directories.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")


# Checkout Git submodules.
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "Updating Git submodules ...")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "Updating Git submodules failed with error code ${GIT_SUBMOD_RESULT}, please checkout submodules manually.")
    endif ()
endif ()


# Configure optional features with a reasonable default value.
option(TRROJAN_WITH_CHAKRA "Enable JavaScript support." OFF)
if (WIN32)
    option(TRROJAN_WITH_DSTORAGE "Enable support for DirectStorage in Direct3D 12 plugin." ON)
else ()
    set(TRROJAN_WITH_DSTORAGE OFF)
endif ()
option(TRROJAN_DEBUG_OVERLAY "Enable overlay in debug view." OFF)

# Set the directory where we expect Chakra Core.
set(CHAKRA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/chakra" CACHE STRING "Directory holding the source of Chakra Core.")

# Set the directory where we expect DirectStorage.
set(DSTORAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Direct3D.DirectStorage.1.0.0" CACHE STRING "Directory holding the DirectStorage APIs.")


# Configure the compiler.
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

if (NOT WIN32 OR ${PNG_FOUND})
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

# Build glm
#set(GLM_STATIC_LIBRARY_ENABLE "YES")
#set(GLM_TEST_ENABLE "NO")
#add_subdirectory(glm)

# Build spdlog
set(SPDLOG_BUILD_TESTING "NO" FORCE)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_PRINTF")
add_subdirectory(spdlog)

# Include headers for all of TRRojan.
include_directories("glm")
include_directories("spdlog/include")
include_directories("datraw/datraw")
include_directories("mmpld/mmpld")

# Build the system information library.
add_subdirectory(trrojansnfo)

# Build the core library
add_subdirectory(trrojancore)

# Build the OpenCL plugin
add_subdirectory(trrojancl)

# Build the RAM stream plugin
add_subdirectory(trrojanstream)

# Build the D3D plugins.
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMMPLD_WITH_DIRECT3D")
    add_subdirectory(trrojand3d11)
    add_subdirectory(trrojand3d12)
endif ()

# Build the executable
add_subdirectory(trrojan)
