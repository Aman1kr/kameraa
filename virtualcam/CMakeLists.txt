# Kameraa DirectShow virtual-camera filter DLL (Windows only, opt-in).
#
# Requires the DirectShow Base Classes ("baseclasses"/strmbase), which ship as
# source with the Windows SDK samples and are not provided by vcpkg. Point
# KAMERAA_STRMBASE_DIR at a compiled strmbase (headers + strmbase.lib), e.g.:
#   -DKAMERAA_BUILD_DSHOW_FILTER=ON -DKAMERAA_STRMBASE_DIR=C:/path/to/baseclasses
#
# See docs/05-PLATFORM-INTEGRATION.md and docs/10-BUILD-AND-PACKAGING.md.

if(NOT WIN32)
    return()
endif()

option(KAMERAA_BUILD_DSHOW_FILTER "Build the DirectShow virtual camera DLL" OFF)
if(NOT KAMERAA_BUILD_DSHOW_FILTER)
    return()
endif()

set(KAMERAA_STRMBASE_DIR "" CACHE PATH "Directory with DirectShow base classes (strmbase)")
if(NOT KAMERAA_STRMBASE_DIR OR NOT EXISTS "${KAMERAA_STRMBASE_DIR}")
    message(FATAL_ERROR
        "KAMERAA_BUILD_DSHOW_FILTER=ON requires KAMERAA_STRMBASE_DIR to point at the "
        "DirectShow base classes (strmbase). See virtualcam/CMakeLists.txt.")
endif()

add_library(kameraa_vcam SHARED
    VCamFilter.cpp
    kameraa_vcam.def
)

target_include_directories(kameraa_vcam PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KAMERAA_STRMBASE_DIR}
)

find_library(STRMBASE_LIB NAMES strmbase strmbasd HINTS "${KAMERAA_STRMBASE_DIR}")
target_link_libraries(kameraa_vcam PRIVATE
    ${STRMBASE_LIB}
    strmiids
    winmm
    ole32
    oleaut32
    uuid
)

set_target_properties(kameraa_vcam PROPERTIES OUTPUT_NAME "kameraa_vcam")
