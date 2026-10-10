add_library(kameraa_capture STATIC
    OpenCvCaptureSource.cpp
    TestPatternSource.cpp
)

target_include_directories(kameraa_capture PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(kameraa_capture PUBLIC kameraa_core ${OpenCV_LIBS})
