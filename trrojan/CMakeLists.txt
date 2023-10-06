# CMakeLists.txt
# Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

include(FindPkgConfig)


project(trrojan)


file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")

# Do some debugging quickhack ...
#include_directories(${TrrojanSysInfoIncludeDir})
#include_directories(${TrrojanStreamIncludeDir})
#target_link_libraries(trrojan LINK_PUBLIC trrojanstream)

# Define the output
add_executable(trrojan ${SourceFiles})
target_link_libraries(trrojan PRIVATE ${SYSTEM_LIBS} trrojancore)


# All the CImg crap by Valentin, but refactored to avoid unnecessary stuff.
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

if (LIBAVCODEC_FOUND  AND LIBAVFORMAT_FOUND AND LIBSWSCALE_FOUND AND LIBAVUTIL_FOUND)
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


#Path of CImg.h file relative to this file path
#set(CIMG_H_PATH ${TrrojanCoreIncludeDir}/lib/)
#include_directories( ${CIMG_H_PATH} )


if (NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32)
endif()


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
