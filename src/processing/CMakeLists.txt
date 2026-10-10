add_library(kameraa_processing STATIC
    DenoiseFilter.cpp
    RelightFilter.cpp
    SharpenFilter.cpp
    OnnxModel.cpp
    ModelRegistry.cpp
    SuperResFilter.cpp
    BackgroundFilter.cpp
    FaceEnhanceFilter.cpp
    FrameInterpolator.cpp
)

target_include_directories(kameraa_processing PUBLIC ${CMAKE_SOURCE_DIR}/src)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(kameraa_processing
    PUBLIC kameraa_core ${OpenCV_LIBS}
    PRIVATE nlohmann_json::nlohmann_json
)

option(KAMERAA_ENABLE_ONNX "Enable ONNX Runtime AI filters" OFF)
if(KAMERAA_ENABLE_ONNX)
    find_package(onnxruntime CONFIG QUIET)
    if(onnxruntime_FOUND)
        target_link_libraries(kameraa_processing PUBLIC onnxruntime::onnxruntime)
        target_compile_definitions(kameraa_processing PRIVATE KAMERAA_WITH_ONNX)
        message(STATUS "Kameraa: ONNX Runtime AI filters ENABLED")

        # Optional GPU execution providers (require an onnxruntime built with them).
        if(KAMERAA_ENABLE_CUDA)
            target_compile_definitions(kameraa_processing PRIVATE KAMERAA_WITH_CUDA)
            message(STATUS "Kameraa: ONNX CUDA execution provider ENABLED")
        endif()
        if(KAMERAA_ENABLE_DIRECTML)
            target_compile_definitions(kameraa_processing PRIVATE KAMERAA_WITH_DIRECTML)
            message(STATUS "Kameraa: ONNX DirectML execution provider ENABLED")
        endif()
        if(KAMERAA_ENABLE_COREML)
            target_compile_definitions(kameraa_processing PRIVATE KAMERAA_WITH_COREML)
            message(STATUS "Kameraa: ONNX CoreML execution provider ENABLED")
        endif()
    else()
        message(WARNING "Kameraa: KAMERAA_ENABLE_ONNX set but onnxruntime not found; "
                        "building without AI filters (classical path still works)")
    endif()
endif()
