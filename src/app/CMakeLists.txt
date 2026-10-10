add_executable(kameraa
    main.cpp
)

target_include_directories(kameraa PRIVATE ${CMAKE_SOURCE_DIR}/src)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(kameraa
    PRIVATE
        kameraa_core
        kameraa_capture
        kameraa_processing
        kameraa_config
        kameraa_output
        kameraa_gui
        Qt6::Widgets
        spdlog::spdlog
)

set_target_properties(kameraa PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(APPLE)
    set_target_properties(kameraa PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/release/macos/Info.plist.in
        MACOSX_BUNDLE_BUNDLE_NAME "Kameraa"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.kameraa.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()
