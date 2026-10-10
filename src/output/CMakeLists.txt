add_library(kameraa_output STATIC
    V4l2LoopbackSink.cpp
    SharedFrameChannel.cpp
    DirectShowSink.cpp
    CmioExtensionSink.cpp
    VirtualCameraFactory.cpp
)

target_include_directories(kameraa_output PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(kameraa_output PUBLIC kameraa_core ${OpenCV_LIBS})
