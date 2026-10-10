#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "processing/ModelRegistry.h"

using namespace kameraa;
namespace fs = std::filesystem;

TEST(ModelRegistryTest, ParsesManifestAndFlagsMissingFiles) {
    const fs::path dir = fs::temp_directory_path() / "kameraa_models_test";
    fs::create_directories(dir);
    {
        std::ofstream mf(dir / "manifest.json");
        mf << R"({
          "models": [
            { "id": "superres", "file": "espcn_x2.onnx", "scale": 2 },
            { "id": "background", "file": "modnet_512.onnx", "inputSize": 512 }
          ]
        })";
    }

    ModelRegistry reg(dir.string());
    ASSERT_TRUE(reg.loadManifest());

    const ModelInfo sr = reg.get("superres");
    EXPECT_EQ(sr.scale, 2);
    EXPECT_FALSE(sr.found);  // file not actually present

    // Create the file -> should now be found.
    { std::ofstream(dir / "espcn_x2.onnx") << "x"; }
    ModelRegistry reg2(dir.string());
    ASSERT_TRUE(reg2.loadManifest());
    EXPECT_TRUE(reg2.get("superres").found);

    EXPECT_FALSE(reg2.get("does_not_exist").found);
    fs::remove_all(dir);
}

TEST(ModelRegistryTest, MissingManifestFails) {
    ModelRegistry reg((fs::temp_directory_path() / "no_such_dir_kameraa").string());
    EXPECT_FALSE(reg.loadManifest());
}
