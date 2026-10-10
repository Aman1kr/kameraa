find_package(GTest CONFIG REQUIRED)

add_executable(kameraa_tests
    test_filters.cpp
    test_settings.cpp
    test_pipeline.cpp
    test_model_registry.cpp
)

target_include_directories(kameraa_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(kameraa_tests
    PRIVATE
        kameraa_core
        kameraa_capture
        kameraa_processing
        kameraa_config
        GTest::gtest
        GTest::gtest_main
        ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(kameraa_tests)
