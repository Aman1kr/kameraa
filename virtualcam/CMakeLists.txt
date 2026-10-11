# Kameraa DirectShow virtual-camera filter DLL (Windows only, opt-in).
#
# Builds kameraa_vcam.dll by compiling the Microsoft DirectShow "base classes"
# (fetched in CI from the Windows-classic-samples repo) together with our filter.
# Enable with: -DKAMERAA_BUILD_DSHOW_FILTER=ON -DKAMERAA_STRMBASE_DIR=<baseclasses>
# Can be built standalone (cmake -S virtualcam) or as a subdirectory of the app.
# See docs/05-PLATFORM-INTEGRATION.md and .github/workflows/windows-installer.yml.

cmake_minimum_required(VERSION 3.21)

# When built on its own, establish a project and default the option on.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(kameraa_vcam LANGUAGES CXX)
    set(KAMERAA_BUILD_DSHOW_FILTER ON)
endif()

if(NOT WIN32)
    return()
endif()

if(NOT DEFINED KAMERAA_BUILD_DSHOW_FILTER)
    option(KAMERAA_BUILD_DSHOW_FILTER "Build the DirectShow virtual camera DLL" OFF)
endif()
if(NOT KAMERAA_BUILD_DSHOW_FILTER)
    return()
endif()

set(KAMERAA_STRMBASE_DIR "" CACHE PATH "Directory with DirectShow base classes sources (strmbase)")
if(NOT KAMERAA_STRMBASE_DIR OR NOT EXISTS "${KAMERAA_STRMBASE_DIR}/streams.h")
    message(FATAL_ERROR
        "KAMERAA_BUILD_DSHOW_FILTER=ON requires KAMERAA_STRMBASE_DIR to point at the "
        "DirectShow base classes source folder (containing streams.h).")
endif()

# Compile the base-class sources directly into the DLL.
file(GLOB BASECLASS_SOURCES "${KAMERAA_STRMBASE_DIR}/*.cpp")

add_library(kameraa_vcam SHARED
    VCamFilter.cpp
    kameraa_vcam.def
    ${BASECLASS_SOURCES}
)

target_include_directories(kameraa_vcam PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KAMERAA_STRMBASE_DIR}
)

# The base classes are legacy code; relax warnings and set required defines.
target_compile_definitions(kameraa_vcam PRIVATE
    _WINDOWS WIN32 _WIN32_WINNT=0x0601 UNICODE _UNICODE)
if(MSVC)
    target_compile_options(kameraa_vcam PRIVATE /W0 /wd4996 /EHsc)
endif()

target_link_libraries(kameraa_vcam PRIVATE
    strmiids
    winmm
    ole32
    oleaut32
    uuid
    user32
    gdi32
    advapi32
    legacy_stdio_definitions
)

set_target_properties(kameraa_vcam PROPERTIES OUTPUT_NAME "kameraa_vcam")
