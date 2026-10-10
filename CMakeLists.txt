cmake_minimum_required(VERSION 3.21)
project(kameraa VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generated version header (included as "kameraa/Version.h").
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/kameraa/Version.h
    @ONLY)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Options (future-facing; wired in later phases)
option(KAMERAA_BUILD_TESTS     "Build unit tests"        OFF)
option(KAMERAA_ENABLE_CUDA     "Enable CUDA EP"          OFF)
option(KAMERAA_ENABLE_DIRECTML "Enable DirectML EP"      OFF)
option(KAMERAA_ENABLE_COREML   "Enable CoreML EP"        OFF)

# Qt (Widgets) — Phase 0 GUI
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# OpenCV — capture + classical imaging
find_package(OpenCV REQUIRED)

add_subdirectory(src)

add_subdirectory(virtualcam)

if(KAMERAA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Install & packaging -----------------------------------------------------
install(TARGETS kameraa
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
install(DIRECTORY models/ DESTINATION share/kameraa/models
    FILES_MATCHING PATTERN "manifest.json")
install(FILES README.md DESTINATION share/kameraa OPTIONAL)

# Linux desktop integration.
if(UNIX AND NOT APPLE)
    install(FILES release/linux/kameraa.desktop
        DESTINATION share/applications)
    install(FILES release/linux/com.kameraa.Kameraa.metainfo.xml
        DESTINATION share/metainfo)
endif()

set(CPACK_PACKAGE_NAME "Kameraa")
set(CPACK_PACKAGE_VENDOR "Kameraa")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AI virtual camera enhancer")
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kameraa")
    # v4l2loopback is a runtime dependency the user must install (kernel module).
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "v4l2loopback-dkms")
endif()
include(CPack)
