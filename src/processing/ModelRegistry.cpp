#include "processing/ModelRegistry.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace kameraa {

ModelRegistry::ModelRegistry(std::string modelsDir)
    : modelsDir_(std::move(modelsDir)) {}

std::string ModelRegistry::defaultModelsDir() {
    if (const char* env = std::getenv("KAMERAA_MODELS_DIR")) {
        return env;
    }
    return "models";
}

bool ModelRegistry::loadManifest() {
    models_.clear();
    const fs::path manifest = fs::path(modelsDir_) / "manifest.json";
    std::ifstream in(manifest);
    if (!in) return false;

    try {
        nlohmann::json j;
        in >> j;
        if (!j.contains("models")) return false;
        for (const auto& m : j.at("models")) {
            ModelInfo info;
            info.id = m.value("id", std::string{});
            info.file = m.value("file", std::string{});
            info.sha256 = m.value("sha256", std::string{});
            info.scale = m.value("scale", 0);
            info.inputSize = m.value("inputSize", 0);
            info.license = m.value("license", std::string{});
            if (info.id.empty() || info.file.empty()) continue;

            const fs::path p = fs::path(modelsDir_) / info.file;
            info.path = p.string();
            std::error_code ec;
            info.found = fs::exists(p, ec) && !ec;
            models_[info.id] = info;
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

ModelInfo ModelRegistry::get(const std::string& id) const {
    auto it = models_.find(id);
    if (it != models_.end()) return it->second;
    ModelInfo missing;
    missing.id = id;
    return missing;
}

}  // namespace kameraa
