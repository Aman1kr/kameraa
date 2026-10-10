add_library(kameraa_core STATIC
    Pipeline.cpp
)

target_include_directories(kameraa_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(kameraa_core PUBLIC ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(kameraa_core PUBLIC ${OpenCV_LIBS})
target_include_directories(kameraa_core PUBLIC ${OpenCV_INCLUDE_DIRS})
