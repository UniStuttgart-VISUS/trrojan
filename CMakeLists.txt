# CMakeLists.txt
# Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

project(TRRojan)

find_package(JPEG)
find_package(TIFF)
find_package(ZLIB)
find_package(PNG)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")


# Configure the compiler.
IF (${CMAKE_COMPILER_IS_GNUCXX})
	add_definitions(-pedantic -fPIC -DUNIX )
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
ENDIF (${CMAKE_COMPILER_IS_GNUCXX})


if (${PNG_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_png")
endif (${PNG_FOUND})

if (${JPEG_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_jpeg")
endif (${JPEG_FOUND})

if (${TIFF_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_tiff")
endif (${TIFF_FOUND})

if (${ZLIB_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_zlib")
endif (${ZLIB_FOUND})

if (NOT WIN32 OR ${PNG_FOUND})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRROJAN_WITH_CIMG -Dcimg_display=0")
endif (NOT WIN32 OR ${PNG_FOUND})

 find_package(OpenMP)
 if (OPENMP_FOUND)
     set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 endif()

########################
# Build the core library
add_subdirectory(trrojancore)

# Build the OpenCL plugin
add_subdirectory(trrojancl)

# Build the RAM stream plugin
add_subdirectory(trrojanstream)

# Build the executable
add_subdirectory(trrojan)
