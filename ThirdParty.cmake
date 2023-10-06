# ThirdParty.cmake
# Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

include(FetchContent)

set(FETCHCONTENT_QUIET ON)
mark_as_advanced(FORCE
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_QUIET
    FETCHCONTENT_UPDATES_DISCONNECTED)


# Chakra Core
#FetchContent_Declare(ChakraCore
#    URL "https://github.com/chakra-core/ChakraCore/archive/refs/tags/v1.11.24.zip"
#)
#FetchContent_MakeAvailable(ChakraCore)
#mark_as_advanced(FORCE
#    FETCHCONTENT_SOURCE_DIR_CHAKRACORE
#    FETCHCONTENT_UPDATES_DISCONNECTED_CHAKRACORE)


# datraw
FetchContent_Declare(datraw
    URL "https://github.com/UniStuttgart-VISUS/datraw/archive/refs/tags/v1.0.6.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(datraw)
add_library(datraw INTERFACE IMPORTED GLOBAL)
set_target_properties(datraw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${datraw_SOURCE_DIR}/datraw")
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_DATRAW
    FETCHCONTENT_UPDATES_DISCONNECTED_DATRAW)


# glm
FetchContent_Declare(glm
    URL "https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(glm)
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_GLM
    FETCHCONTENT_UPDATES_DISCONNECTED_GLM)


# mmpld
FetchContent_Declare(mmpld
    URL "https://github.com/UniStuttgart-VISUS/mmpld/archive/refs/tags/v1.16.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(mmpld)
add_library(mmpld INTERFACE IMPORTED GLOBAL)
set_target_properties(mmpld PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${mmpld_SOURCE_DIR}/mmpld")
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_MMPLD
    FETCHCONTENT_UPDATES_DISCONNECTED_MMPLD)


# Power Overwhelming
FetchContent_Declare(pwrowg
    URL "https://github.com/UniStuttgart-VISUS/power-overwhelming/archive/refs/tags/v1.8.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(pwrowg)
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_PWROWG
    FETCHCONTENT_UPDATES_DISCONNECTED_PWROWG)


# spdlog
FetchContent_Declare(spdlog
    URL "https://github.com/gabime/spdlog/archive/refs/tags/v0.17.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
option(SPDLOG_BUILD_TESTING "" OFF)
option(SPDLOG_BUILD_EXAMPLES "" OFF)
FetchContent_MakeAvailable(spdlog)
mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_SPDLOG
    FETCHCONTENT_UPDATES_DISCONNECTED_SPDLOG
    SPDLOG_BUILD_TESTING
    SPDLOG_BUILD_EXAMPLES)
