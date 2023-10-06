# CMakeLists.txt
# Copyright (C) 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

project(trrojancore)


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
target_compile_definitions(${PROJECT_NAME} PRIVATE TRROJANCORE_EXPORTS)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${IncludeDirectory}>
    PRIVATE
        $<BUILD_INTERFACE:${SourceDirectory}>)

target_link_libraries(${PROJECT_NAME} PUBLIC trrojansnfo)
target_link_libraries(${PROJECT_NAME} PUBLIC datraw)
target_link_libraries(${PROJECT_NAME} PUBLIC mmpld)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

if (${CMAKE_COMPILER_IS_GNUCXX})
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif ()

if (TRROJAN_WITH_POWER_OVERWHELMING)
    target_link_libraries(${PROJECT_NAME} PRIVATE power_overwhelming)
    #add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${POWER_OVERWHELMING_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/power_overwhelming.dll" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
endif ()

if (TRROJAN_WITH_DSTORAGE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dstorage)
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
