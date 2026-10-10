#pragma once

#include <map>
#include <string>

namespace kameraa {

struct ModelInfo {
    std::string id;
    std::string file;
    std::string sha256;  // expected hash (verification is a later hardening step)
    int scale = 0;       // super-resolution factor, if applicable
    int inputSize = 0;   // square input size, if applicable (e.g. matting)
    std::string license;
    bool found = false;  // file exists on disk
    std::string path;    // resolved absolute/relative path
};

// Resolves AI model files described in models/manifest.json. Models are optional:
// when a file is absent the owning filter stays disabled (classical path still works).
class ModelRegistry {
public:
    explicit ModelRegistry(std::string modelsDir = defaultModelsDir());

    bool loadManifest();
    ModelInfo get(const std::string& id) const;
    const std::string& modelsDir() const { return modelsDir_; }

    // models dir: $KAMERAA_MODELS_DIR, else "models" next to the CWD.
    static std::string defaultModelsDir();

private:
    std::string modelsDir_;
    std::map<std::string, ModelInfo> models_;
};

}  // namespace kameraa
