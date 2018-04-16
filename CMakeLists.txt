# CMakeLists.txt
# Copyright © 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

project(TRRojan)

find_package(JPEG)
find_package(TIFF)
find_package(ZLIB)
find_package(PNG)
find_package(OpenMP)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ????????????
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")


# Configure optional features
option(TRROJAN_WITH_CHAKRA "Enable JavaScript support." OFF)


# Configure the compiler.
if (WIN32)
	add_compile_options(/wd4251)	# We cannot fix that - TRRojan just needs to be compiled in a single step.
endif (WIN32)

if (${CMAKE_COMPILER_IS_GNUCXX})
	add_definitions(-pedantic -fPIC -DUNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
endif (${CMAKE_COMPILER_IS_GNUCXX})
if (${MSVC})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -EHsc")
endif (${MSVC})

if (NOT WIN32 OR ${PNG_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAN_WITH_CIMG -Dcimg_display=0")
endif (NOT WIN32 OR ${PNG_FOUND})

if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


# Set the directory where we expect Chakra Core.
set(CHAKRA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/chakra" CACHE STRING "Directory holding the source of Chakra Core.")

# Build glm
#set(GLM_STATIC_LIBRARY_ENABLE "YES")
#set(GLM_TEST_ENABLE "NO")
#add_subdirectory(glm)

# Build spdlog
set(SPDLOG_BUILD_TESTING "NO" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_PRINTF")
add_subdirectory(spdlog)


# Include headers for all of TRRojan.
include_directories("glm")
include_directories("spdlog/include")
include_directories("datraw/datraw")

# Build the system information library.
add_subdirectory(trrojansnfo)

# Build the core library
add_subdirectory(trrojancore)

# Build the OpenCL plugin
add_subdirectory(trrojancl)

# Build the RAM stream plugin
add_subdirectory(trrojanstream)

# Build the D3D plugin
if (WIN32)
    add_subdirectory(trrojand3d11)
endif (WIN32)

# Build the executable
add_subdirectory(trrojan)
