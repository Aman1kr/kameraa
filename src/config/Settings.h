#pragma once

#include <string>

namespace kameraa {

// Serializable enhancement parameters (maps 1:1 to the classical filters).
struct FilterSettings {
    bool denoiseEnabled = true;
    double denoiseStrength = 0.4;

    bool relightEnabled = true;
    bool autoLight = true;
    double brightness = 0.0;
    double contrast = 0.0;
    double saturation = 0.0;

    bool sharpenEnabled = true;
    double sharpenAmount = 0.5;

    bool superresEnabled = false;
    int superresScale = 2;

    bool backgroundEnabled = false;
    int backgroundMode = 0;       // 0 = blur, 1 = replace
    double backgroundStrength = 0.6;

    bool faceEnabled = false;
    double faceStrength = 0.5;

    bool interpEnabled = false;
};

struct Settings {
    std::string preset = "Balanced";
    int cameraIndex = 0;
    bool virtualCameraOutput = true;
    std::string accel = "Auto";  // Auto | CPU | CUDA | DirectML | CoreML
    FilterSettings filters;
};

// Built-in presets (see docs/04-PIPELINE-DESIGN.md).
FilterSettings presetLite();
FilterSettings presetBalanced();
FilterSettings presetStudio();
FilterSettings presetByName(const std::string& name);

// JSON persistence. Returns false on IO/parse error (settings left unchanged).
bool loadSettings(const std::string& path, Settings& out);
bool saveSettings(const std::string& path, const Settings& in);

// Default per-user settings file location.
std::string defaultSettingsPath();

}  // namespace kameraa
