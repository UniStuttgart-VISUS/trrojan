# CMakeLists.txt
# Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

project(trrojan)
include(FindPkgConfig)


file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
if (WIN32)
    file(GLOB_RECURSE ResourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.rc")
else ()
    set(ResourceFiles "")
endif ()

# Files that are embedded as assets in the APPX package.
if (TRROJAN_FOR_UWP)
    set(AssetFiles
        Assets/demo_d11.trroll
        Assets/demo_d12.trroll
        Assets/spheres.trroll
        Assets/Logo.png
        Assets/SmallLogo.png
        Assets/SmallLogo44x44.png
        Assets/SplashScreen.png
        Assets/StoreLogo.png
        Assets/mmpld.trroll
        Assets/testspheres_old.mmpld
        #Assets/coronadir_16_100.mmpld
        #Assets/laser.charge.00104.mmpld
        Assets/volume-tf_demo.trroll
        #Assets/foot.dat
        #Assets/foot.raw
        Assets/bonsai.dat
        Assets/bonsai.raw
        Assets/trrojan_cham1.brudervn
        Assets/trrojan_zeiss1.brudervn
        Assets/trrojan_zeiss2.brudervn
    )
endif()

# Files that are packaged as content in the APPX package
if (TRROJAN_FOR_UWP)
    set(ContentFiles package.appxmanifest)
endif ()

# Additional UWP resources
if (TRROJAN_FOR_UWP)
    set(ResourceFiles ${ResourceFiles} ${ContentFiles} ${AssetFiles} Trrojan_TemporaryKey.pfx)
endif ()

# Tag the resource accordingly for the UWP build
if (TRROJAN_FOR_UWP)
    set_property(SOURCE ${ContentFiles} PROPERTY VS_DEPLOYMENT_CONTENT 1)
    set_property(SOURCE ${AssetFiles} PROPERTY VS_DEPLOYMENT_CONTENT 1)
    set_property(SOURCE ${AssetFiles} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
endif ()


# Define the output (note that UWP is using the Win32 subsystem, not console)
if (TRROJAN_FOR_UWP)
    add_executable(trrojan WIN32 ${HeaderFiles} ${SourceFiles} ${ResourceFiles})
else ()
    add_executable(trrojan ${HeaderFiles} ${SourceFiles} ${ResourceFiles})
endif ()
target_link_libraries(trrojan PRIVATE ${SYSTEM_LIBS} trrojancore)


# Group resource scripts into a new filter.
source_group("Resource Files" FILES ${ResourceFiles})


# All the CImg crap Valentin needs to save screenies on Linux.
#find_package(OpenCV)
find_package(JPEG)
find_package(TIFF)
find_package(PNG)
find_package(ZLIB)
find_package(LAPACK)
find_package(BLAS)

if (NOT WIN32)
    find_package(X11)
    find_package(Threads REQUIRED)
endif ()

pkg_check_modules(FFTW3 fftw3)
pkg_check_modules(OPENEXR OpenEXR)
pkg_check_modules(MAGICK Magick++)

if (TIFF_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_tiff)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${TIFF_LIBRARIES})
endif ()

if (JPEG_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_jpeg)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JPEG_LIBRARIES})
endif ()

if (ZLIB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})

    # PNG requires ZLIB
    if (PNG_FOUND)
        target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_png)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${PNG_LIBRARIES})
    endif ()
endif ()

if (FFTW3_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_fftw3)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFTW3_LIBRARIES})
endif ()

if (OPENEXR_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_openexr)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENEXR_LIBRARIES})
endif ()

if (MAGICK_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_magick)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MAGICK_LIBRARIES})
endif()

if (LIBAVCODEC_FOUND AND LIBAVFORMAT_FOUND AND LIBSWSCALE_FOUND AND LIBAVUTIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_ffmpeg)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBAVFORMAT_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBAVCODEC_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBSWSCALE_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBAVUTIL_LIBRARIES})
#    include_directories( ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS}/libavformat)
#    include_directories( ${LIBAVCODEC_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS}/libavcodec )
#    include_directories( ${LIBSWSCALE_INCLUDE_DIRS} ${LIBSWSCALE_INCLUDE_DIRS}/libswscale)
#    include_directories( ${LIBAVUTIL_INCLUDE_DIRS} ${LIBAVUTIL_INCLUDE_DIRS}/libavutil )
endif ()

if ((NOT APPLE) AND (NOT WIN32) AND X11_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE cimg_use_xshm cimg_use_xrand)
    target_link_libraries(${PROJECT_NAME} PRIVATE Xext Xrandr)
endif ()

if (X11_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
endif ()

if (NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32)
endif()

# On UWP, we need to link all the DLLs, because we need to deploy each of them
# in the APPX package. UWP cannot load DLLs that are not packaged.
if (TRROJAN_FOR_UWP)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${TRROJAN_PLUGINS})
endif ()

#Path of CImg.h file relative to this file path
#set(CIMG_H_PATH ${TrrojanCoreIncludeDir}/lib/)
#include_directories( ${CIMG_H_PATH} )


# Make sure that the user-specified UAP version is set.
if (TRROJAN_FOR_UWP)
    set_target_properties(trrojan PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION ${TRROJAN_UWP_PLATFORM_VERSION})
endif ()



## Ensure the right build order such that the following debugging copy
## operations will work.
#if (TRROJAN_WITH_OPENCL)
#    add_dependencies(trrojan trrojancl)
#endif ()
#add_dependencies(trrojan trrojanstream)
#if (WIN32)
#    add_dependencies(trrojan trrojand3d11)
#    add_dependencies(trrojan trrojand3d12)
#endif ()

# For debugging the executable directly from Visual Studio, copy all of the
# benchmark DLLs to the output directory.
set(TRROJAN_INTERNAL_DEPENDENCIES trrojancore ${TRROJAN_PLUGINS})
if (NOT TRROJAN_FOR_UWP)
    set(TRROJAN_INTERNAL_DEPENDENCIES "${TRROJAN_INTERNAL_DEPENDENCIES}" trrojansnfo)
endif()

foreach (TRROJAN_INT_DEP ${TRROJAN_INTERNAL_DEPENDENCIES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${${TRROJAN_INT_DEP}_BINARY_DIR}/$<CONFIG>/${TRROJAN_INT_DEP}${CMAKE_SHARED_LIBRARY_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
endforeach ()

if (TRROJAN_WITH_CHAKRA)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${trrojancore_BINARY_DIR}/$<CONFIG>/chakracore${CMAKE_SHARED_LIBRARY_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
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
