# CMakeLists.txt
# Copyright (C) 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

project(trrojancore)


# Glob and add sources and resources
set(IncludeDirectory "include")
set(SourceDirectory "src")

file(GLOB_RECURSE PublicHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${IncludeDirectory}/*.h" "${IncludeDirectory}/*.inl")
file(GLOB_RECURSE PrivateHeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.h" "${SourceDirectory}/*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.cpp")

if (WIN32)
    file(GLOB_RECURSE ResourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SourceDirectory}/*.rc")
else ()
    set(ResourceFiles "")
endif ()


# Define the library target
add_library(${PROJECT_NAME} SHARED ${PublicHeaderFiles} ${PrivateHeaderFiles} ${SourceFiles} ${ResourceFiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${IncludeDirectory})
target_compile_definitions(${PROJECT_NAME} PRIVATE TRROJANCORE_EXPORTS)

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

    else ()
        set (CHAKRA_BIN_DIR "${CHAKRA_DIR}/out/${CHAKRA_CONFIGURATION}")
        set (CHAKRA_LIB "${CHAKRA_BIN_DIR}/libChakraCore.so")

        add_custom_command(OUTPUT ${CHAKRA_LIB}
                           COMMAND ${CHAKRA_DIR}/build.sh
                           COMMENT "Building Chakra Core ...")
        add_custom_target(chakra DEPENDS ${CHAKRA_LIB})
        add_dependencies(trrojancore chakra)

        add_library(ChakraCore STATIC IMPORTED)
        set_property(TARGET ChakraCore PROPERTY IMPORTED_LOCATION ${CHAKRA_LIB})

        target_link_libraries(trrojancore ChakraCore)
    endif ()
endif ()

# Configure the linker
target_link_libraries(${PROJECT_NAME} trrojansnfo)

if (${CMAKE_COMPILER_IS_GNUCXX})
    target_link_libraries(${PROJECT_NAME} dl)
endif ()

if (TRROJAN_WITH_POWER_OVERWHELMING)
    target_link_libraries(${PROJECT_NAME} power_overwhelming)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${POWER_OVERWHELMING_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/power_overwhelming.dll" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
endif ()

if (TRROJAN_WITH_DSTORAGE)
    target_link_libraries(${PROJECT_NAME} dstorage)
    #install(FILES "${DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/dstorage.dll" DESTINATION bin)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${DSTORAGE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/dstorage.dll" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
endif ()


# Installation
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${IncludeDirectory}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})


# TODO: Remove this
set(TrrojanCoreIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/${IncludeDirectory}" PARENT_SCOPE)
