# CMakeLists.txt
# Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

project(TRRojan)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_USE_RELATIVE_PATHS ON)


# Configure the compiler.
IF (${CMAKE_COMPILER_IS_GNUCXX})
	add_definitions(-pedantic -fPIC -DUNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
ENDIF (${CMAKE_COMPILER_IS_GNUCXX})


# Build the core library
add_subdirectory(trrojancore)

# Build the executable
add_subdirectory(trrojan)
