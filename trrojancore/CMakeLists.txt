# CMakeLists.txt
# Copyright (C) 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

set(IncludeDirectory, "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SourceDirectory, "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE HeaderFiles RELATIVE "${IncludeDirectory}" "*.h")
file(GLOB_RECURSE SourceFiles RELATIVE "${SourceDirectory}" "*.cpp")

add_library(trrojancore STATIC ${HeaderFiles} ${SourceFiles})

target_include_directories(trrojancore PUBLIC ${IncludeDirectory})

include_directories(${IncludeDirectory})

# Configure the compiler
IF (${CMAKE_COMPILER_IS_GNUCXX})
	add_definitions(-pedantic -fPIC -DUNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
ENDIF (${CMAKE_COMPILER_IS_GNUCXX})


# Enable compiler features
#target_compile_features(trrojancore PUBLIC cxx_alias_templates)
#target_compile_features(trrojancore PUBLIC cxx_attribute_deprecated)
#target_compile_features(trrojancore PUBLIC cxx_auto_type cxx_decltype)
#target_compile_features(trrojancore PUBLIC cxx_func_identifier)
#target_compile_features(trrojancore PUBLIC cxx_lambdas)
#target_compile_features(trrojancore PUBLIC cxx_range_for)
#target_compile_features(trrojancore PUBLIC cxx_static_assert)
#target_compile_features(trrojancore PUBLIC cxx_strong_enums)
#target_compile_features(trrojancore PUBLIC cxx_variadic_macros)
#target_compile_features(trrojancore PUBLIC cxx_variadic_templates)
