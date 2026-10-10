add_library(kameraa_config STATIC
    Settings.cpp
)

target_include_directories(kameraa_config PUBLIC ${CMAKE_SOURCE_DIR}/src)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(kameraa_config PUBLIC nlohmann_json::nlohmann_json)
