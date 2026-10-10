#pragma once

#include <string>
#include <vector>

namespace kameraa {

struct CameraInfo {
    int index = -1;
    std::string name;
};

struct CaptureConfig {
    int index = 0;
    int width = 1280;
    int height = 720;
    int fps = 30;
};

struct OutputConfig {
    int width = 1920;
    int height = 1080;
    int fps = 30;
    std::string format = "BGR";  // sink negotiates actual (e.g. NV12/YUY2)
    std::string label = "Kameraa";
};

}  // namespace kameraa
