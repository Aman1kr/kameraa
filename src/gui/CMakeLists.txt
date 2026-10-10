add_library(kameraa_gui STATIC
    MainWindow.cpp
)

target_include_directories(kameraa_gui PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(kameraa_gui
    PUBLIC
        kameraa_core
        kameraa_capture
        kameraa_processing
        kameraa_config
        kameraa_output
        Qt6::Widgets
        ${OpenCV_LIBS}
)
