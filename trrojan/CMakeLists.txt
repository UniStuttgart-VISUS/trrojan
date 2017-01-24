# CMakeLists.txt
# Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.1.0)

file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"  "*.h")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"  "*.cpp")

include_directories(${TrrojanCoreIncludeDir})

add_executable(trrojan ${SourceFiles})

target_link_libraries(trrojan LINK_PUBLIC trrojancore)

# Do some debugging quickhack ...
#include_directories(${TrrojanStreamIncludeDir})
#target_link_libraries(trrojan LINK_PUBLIC trrojanstream)


# CImg related
# Flags to enable native support for JPEG image files, using the JPEG library.
# ( http://www.ijg.org/ )
SET(CIMG_JPEG_CCFLAGS  -Dcimg_use_jpeg)
# Flags to enable native support for TIFF image files, using the TIFF library.
# ( http://www.libtiff.org/ )
SET(CIMG_TIFF_CCFLAGS  -Dcimg_use_tiff)
# Flags to enable native support for PNG image files, using the PNG library.
# ( http://www.libpng.org/ )
SET(CIMG_PNG_CCFLAGS -Dcimg_use_png)
# Flags to enable native support of most classical image file formats, using the Magick++ library.
# ( http://www.imagemagick.org/Magick++/ )
SET(CIMG_MAGICK_CCFLAGS -Dcimg_use_magick)

#### Search Additional Libraries ##########
FIND_PACKAGE(JPEG)
FIND_PACKAGE(TIFF)
FIND_PACKAGE(PNG)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(MAGICK Magick++)

#Path of CImg.h file relative to this file path
set(CIMG_H_PATH ${CMAKE_SOURCE_DIR}/lib/)
include_directories( ${CIMG_H_PATH} )

if(NOT WIN32)
  FIND_PACKAGE(X11)
  FIND_PACKAGE(Threads REQUIRED)
endif()

### Configure Paths according to detected packages
if(TIFF_FOUND)
  get_filename_component(TIFF_LIB_DIRS ${TIFF_LIBRARIES} PATH)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_TIFF_CCFLAGS}")
  #link_directories(${TIFF_LIB_DIRS})
  include_directories(${TIFF_INCLUDE_DIR})
  target_link_libraries(trrojan LINK_PUBLIC ${TIFF_LIB_DIRS})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${TIFF_LIBRARIES})
endif()

if(JPEG_FOUND)
  get_filename_component(JPEG_LIB_DIRS ${JPEG_LIBRARIES} PATH)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_JPEG_CCFLAGS}")
  #link_directories(${JPEG_LIB_DIRS})
  include_directories(${JPEG_INCLUDE_DIR})
  target_link_libraries(trrojan LINK_PUBLIC ${JPEG_LIB_DIRS})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${JPEG_LIBRARIES})
endif()

if (ZLIB_FOUND)
  SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_ZLIB_CCFLAGS}")
  #link_directories(${ZLIB_LIB_DIRS})
  include_directories(${ZLIB_INCLUDE_DIR})
  target_link_libraries(trrojan LINK_PUBLIC ${ZLIB_LIBRARY})
  SET(SYSTEM_LIBS ${SYSTEM_LIBS} ${ZLIB_LIBRARIES})

  # PNG requires ZLIB
  if(PNG_FOUND)
    SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_PNG_CCFLAGS}")
    #link_directories(${PNG_LIB_DIRS})
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(trrojan LINK_PUBLIC ${PNG_LIBRARY})
    SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${PNG_LIBRARIES} )
  endif()
endif()

if(NOT APPLE)
  if(NOT WIN32)
    if(X11_FOUND)
      SET(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_XSHM_CCFLAGS} ${CIMG_XRANDR_CCFLAGS}")
      SET(SYSTEM_LIBS ${SYSTEM_LIBS} Xext Xrandr)
    endif()
  endif(NOT WIN32)
endif(NOT APPLE)

if(X11_FOUND)
  link_directories(${X11_LIB_DIRS})
  include_directories(${X11_INCLUDE_DIR})
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${X11_LIBRARIES} )
endif()

if (NOT WIN32)
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
endif()

if( WIN32)
  SET( SYSTEM_LIBS  ${SYSTEM_LIBS}  gdi32 )
endif()

# Add CIMG Flags to Compilation Flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CIMG_CFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CIMG_CFLAGS}")
