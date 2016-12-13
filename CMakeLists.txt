# CMakeLists.txt
# Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

project(TRRojan)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Build the core library
add_subdirectory(trrojancore)

# Build the executable
add_subdirectory(trrojan)
