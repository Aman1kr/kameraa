#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace kameraa {

// Thin wrapper around an ONNX Runtime session for a single-input / single-output
// float NCHW model. Hidden behind a pimpl so the header has no ONNX dependency;
// when built without KAMERAA_WITH_ONNX every call is a safe no-op (not loaded).
class OnnxModel {
public:
    // Preferred hardware accelerator. The wrapper tries it and falls back to CPU.
    enum class Accel { Auto, Cpu, Cuda, DirectML, CoreML };

    OnnxModel();
    ~OnnxModel();
    OnnxModel(const OnnxModel&) = delete;
    OnnxModel& operator=(const OnnxModel&) = delete;

    static void setGlobalAccel(Accel accel);  // process-wide default
    static Accel globalAccel();

    // Loads a model file. Returns false if unavailable (missing file / no ONNX).
    bool load(const std::string& path, int intraOpThreads = 2);
    bool isLoaded() const { return loaded_; }
    // Accelerator actually in use after load ("CPU", "CUDA", "DirectML", "CoreML").
    std::string activeAccel() const { return activeAccel_; }

    // Runs inference. input is NCHW floats matching inShape. On success fills
    // output + outShape. Returns false if not loaded or on inference error.
    bool run(const std::vector<float>& input, const std::array<int64_t, 4>& inShape,
             std::vector<float>& output, std::array<int64_t, 4>& outShape);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool loaded_ = false;
    std::string activeAccel_ = "CPU";
};

}  // namespace kameraa
