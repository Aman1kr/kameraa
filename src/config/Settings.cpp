#include "config/Settings.h"

#include <cstdlib>
#include <fstream>

#include <nlohmann/json.hpp>

namespace kameraa {

using nlohmann::json;

FilterSettings presetLite() {
    FilterSettings f;
    f.denoiseEnabled = true;  f.denoiseStrength = 0.25;
    f.relightEnabled = true;  f.autoLight = true;
    f.brightness = 0.0;       f.contrast = 0.05;  f.saturation = 0.05;
    f.sharpenEnabled = true;  f.sharpenAmount = 0.3;
    f.superresEnabled = false;
    f.backgroundEnabled = false;
    return f;
}

FilterSettings presetBalanced() {
    FilterSettings f;
    f.denoiseEnabled = true;  f.denoiseStrength = 0.4;
    f.relightEnabled = true;  f.autoLight = true;
    f.brightness = 0.0;       f.contrast = 0.1;   f.saturation = 0.1;
    f.sharpenEnabled = true;  f.sharpenAmount = 0.5;
    f.superresEnabled = true; f.superresScale = 2;
    f.backgroundEnabled = true; f.backgroundMode = 0; f.backgroundStrength = 0.6;
    return f;
}

FilterSettings presetStudio() {
    FilterSettings f;
    f.denoiseEnabled = true;  f.denoiseStrength = 0.6;
    f.relightEnabled = true;  f.autoLight = true;
    f.brightness = 0.05;      f.contrast = 0.15;  f.saturation = 0.15;
    f.sharpenEnabled = true;  f.sharpenAmount = 0.7;
    f.superresEnabled = true; f.superresScale = 2;
    f.backgroundEnabled = true; f.backgroundMode = 1; f.backgroundStrength = 0.7;
    f.faceEnabled = true; f.faceStrength = 0.5;
    f.interpEnabled = true;
    return f;
}

FilterSettings presetByName(const std::string& name) {
    if (name == "Lite") return presetLite();
    if (name == "Studio") return presetStudio();
    return presetBalanced();
}

bool loadSettings(const std::string& path, Settings& out) {
    std::ifstream in(path);
    if (!in) return false;
    try {
        json j;
        in >> j;
        out.preset = j.value("preset", out.preset);
        out.cameraIndex = j.value("cameraIndex", out.cameraIndex);
        out.virtualCameraOutput = j.value("virtualCameraOutput", out.virtualCameraOutput);
        out.accel = j.value("accel", out.accel);
        if (j.contains("filters")) {
            const auto& jf = j.at("filters");
            FilterSettings& f = out.filters;
            f.denoiseEnabled = jf.value("denoiseEnabled", f.denoiseEnabled);
            f.denoiseStrength = jf.value("denoiseStrength", f.denoiseStrength);
            f.relightEnabled = jf.value("relightEnabled", f.relightEnabled);
            f.autoLight = jf.value("autoLight", f.autoLight);
            f.brightness = jf.value("brightness", f.brightness);
            f.contrast = jf.value("contrast", f.contrast);
            f.saturation = jf.value("saturation", f.saturation);
            f.sharpenEnabled = jf.value("sharpenEnabled", f.sharpenEnabled);
            f.sharpenAmount = jf.value("sharpenAmount", f.sharpenAmount);
            f.superresEnabled = jf.value("superresEnabled", f.superresEnabled);
            f.superresScale = jf.value("superresScale", f.superresScale);
            f.backgroundEnabled = jf.value("backgroundEnabled", f.backgroundEnabled);
            f.backgroundMode = jf.value("backgroundMode", f.backgroundMode);
            f.backgroundStrength = jf.value("backgroundStrength", f.backgroundStrength);
            f.faceEnabled = jf.value("faceEnabled", f.faceEnabled);
            f.faceStrength = jf.value("faceStrength", f.faceStrength);
            f.interpEnabled = jf.value("interpEnabled", f.interpEnabled);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool saveSettings(const std::string& path, const Settings& in) {
    const FilterSettings& f = in.filters;
    json j = {
        {"preset", in.preset},
        {"cameraIndex", in.cameraIndex},
        {"virtualCameraOutput", in.virtualCameraOutput},
        {"accel", in.accel},
        {"filters",
         {{"denoiseEnabled", f.denoiseEnabled},
          {"denoiseStrength", f.denoiseStrength},
          {"relightEnabled", f.relightEnabled},
          {"autoLight", f.autoLight},
          {"brightness", f.brightness},
          {"contrast", f.contrast},
          {"saturation", f.saturation},
          {"sharpenEnabled", f.sharpenEnabled},
          {"sharpenAmount", f.sharpenAmount},
          {"superresEnabled", f.superresEnabled},
          {"superresScale", f.superresScale},
          {"backgroundEnabled", f.backgroundEnabled},
          {"backgroundMode", f.backgroundMode},
          {"backgroundStrength", f.backgroundStrength},
          {"faceEnabled", f.faceEnabled},
          {"faceStrength", f.faceStrength},
          {"interpEnabled", f.interpEnabled}}}};
    std::ofstream os(path);
    if (!os) return false;
    os << j.dump(2);
    return static_cast<bool>(os);
}

std::string defaultSettingsPath() {
#if defined(_WIN32)
    const char* base = std::getenv("APPDATA");
    const std::string dir = base ? std::string(base) : ".";
    return dir + "\\Kameraa\\settings.json";
#else
    const char* base = std::getenv("HOME");
    const std::string dir = base ? std::string(base) : ".";
    return dir + "/.config/kameraa/settings.json";
#endif
}

}  // namespace kameraa
