#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <string>

#include "config/Settings.h"

using namespace kameraa;

TEST(SettingsTest, PresetsDifferAsExpected) {
    EXPECT_FALSE(presetLite().superresEnabled);
    EXPECT_TRUE(presetBalanced().superresEnabled);
    EXPECT_TRUE(presetStudio().faceEnabled);
    EXPECT_EQ(presetByName("Studio").backgroundMode, 1);
    EXPECT_EQ(presetByName("unknown").denoiseStrength, presetBalanced().denoiseStrength);
}

TEST(SettingsTest, SaveLoadRoundTrip) {
    Settings in;
    in.preset = "Studio";
    in.cameraIndex = 3;
    in.accel = "CUDA";
    in.filters = presetStudio();
    in.filters.brightness = 0.25;
    in.filters.faceStrength = 0.33;

    const auto path = (std::filesystem::temp_directory_path() /
                       "kameraa_settings_test.json").string();
    ASSERT_TRUE(saveSettings(path, in));

    Settings out;
    ASSERT_TRUE(loadSettings(path, out));
    EXPECT_EQ(out.preset, "Studio");
    EXPECT_EQ(out.cameraIndex, 3);
    EXPECT_EQ(out.accel, "CUDA");
    EXPECT_DOUBLE_EQ(out.filters.brightness, 0.25);
    EXPECT_DOUBLE_EQ(out.filters.faceStrength, 0.33);
    EXPECT_EQ(out.filters.backgroundMode, 1);

    std::remove(path.c_str());
}

TEST(SettingsTest, LoadMissingFileFails) {
    Settings out;
    EXPECT_FALSE(loadSettings("/nonexistent/path/kameraa.json", out));
}
