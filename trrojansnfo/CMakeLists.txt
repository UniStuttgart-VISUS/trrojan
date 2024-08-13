# CMakeLists.txt
# Copyright (C) 2018 - 2023 Visualisierungsinstitut der Universität Stuttgart.

project(trrojansnfo)


# Glob and add sources and resources
set(IncludeDirectory "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SourceDirectory "${CMAKE_CURRENT_SOURCE_DIR}/src")

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
target_compile_definitions(${PROJECT_NAME} PRIVATE TRROJANSNFO_EXPORTS)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${IncludeDirectory}>
    PRIVATE
        $<BUILD_INTERFACE:${SourceDirectory}>)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} d3d11 dxgi dxguid setupapi)
endif ()


# Group resource scripts into a new filter
source_group("Resource Files" FILES ${ResourceFiles})


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
