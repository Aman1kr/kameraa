#include "processing/OnnxModel.h"

namespace kameraa {

namespace {
OnnxModel::Accel& globalAccelRef() {
    static OnnxModel::Accel accel = OnnxModel::Accel::Auto;
    return accel;
}
}  // namespace

void OnnxModel::setGlobalAccel(Accel accel) { globalAccelRef() = accel; }
OnnxModel::Accel OnnxModel::globalAccel() { return globalAccelRef(); }

}  // namespace kameraa

#if defined(KAMERAA_WITH_ONNX)

#include <onnxruntime_cxx_api.h>
#if defined(KAMERAA_WITH_DIRECTML)
#include <dml_provider_factory.h>
#endif
#if defined(KAMERAA_WITH_COREML)
#include <coreml_provider_factory.h>
#endif

namespace kameraa {

namespace {
// One shared environment for the whole process.
Ort::Env& ortEnv() {
    static Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "kameraa");
    return env;
}

// Appends the preferred execution provider; returns the name actually applied.
// Any provider that fails to append is skipped, leaving CPU as the fallback.
std::string appendProvider(Ort::SessionOptions& opts, OnnxModel::Accel accel) {
    const bool autoMode = accel == OnnxModel::Accel::Auto;

#if defined(KAMERAA_WITH_CUDA)
    if (autoMode || accel == OnnxModel::Accel::Cuda) {
        try {
            OrtCUDAProviderOptions cuda{};
            opts.AppendExecutionProvider_CUDA(cuda);
            return "CUDA";
        } catch (const std::exception&) {
        }
    }
#endif
#if defined(KAMERAA_WITH_DIRECTML)
    if (autoMode || accel == OnnxModel::Accel::DirectML) {
        try {
            opts.DisableMemPattern();
            opts.SetExecutionMode(ORT_SEQUENTIAL);
            Ort::ThrowOnError(
                OrtSessionOptionsAppendExecutionProvider_DML(opts, 0));
            return "DirectML";
        } catch (const std::exception&) {
        }
    }
#endif
#if defined(KAMERAA_WITH_COREML)
    if (autoMode || accel == OnnxModel::Accel::CoreML) {
        try {
            Ort::ThrowOnError(
                OrtSessionOptionsAppendExecutionProvider_CoreML(opts, 0));
            return "CoreML";
        } catch (const std::exception&) {
        }
    }
#endif
    (void)accel;
    (void)autoMode;
    return "CPU";
}
}  // namespace

struct OnnxModel::Impl {
    std::unique_ptr<Ort::Session> session;
    std::string inputName;
    std::string outputName;
};

OnnxModel::OnnxModel() : impl_(std::make_unique<Impl>()) {}
OnnxModel::~OnnxModel() = default;

bool OnnxModel::load(const std::string& path, int intraOpThreads) {
    try {
        Ort::SessionOptions opts;
        opts.SetIntraOpNumThreads(intraOpThreads > 0 ? intraOpThreads : 1);
        opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        activeAccel_ = appendProvider(opts, globalAccel());

#if defined(_WIN32)
        std::wstring wpath(path.begin(), path.end());
        impl_->session = std::make_unique<Ort::Session>(ortEnv(), wpath.c_str(), opts);
#else
        impl_->session = std::make_unique<Ort::Session>(ortEnv(), path.c_str(), opts);
#endif

        Ort::AllocatorWithDefaultOptions alloc;
        impl_->inputName = impl_->session->GetInputNameAllocated(0, alloc).get();
        impl_->outputName = impl_->session->GetOutputNameAllocated(0, alloc).get();

        loaded_ = true;
        return true;
    } catch (const std::exception&) {
        impl_->session.reset();
        loaded_ = false;
        activeAccel_ = "CPU";
        return false;
    }
}

bool OnnxModel::run(const std::vector<float>& input,
                    const std::array<int64_t, 4>& inShape,
                    std::vector<float>& output,
                    std::array<int64_t, 4>& outShape) {
    if (!loaded_ || !impl_->session) return false;
    try {
        Ort::MemoryInfo memInfo =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memInfo, const_cast<float*>(input.data()), input.size(),
            inShape.data(), inShape.size());

        const char* inNames[] = {impl_->inputName.c_str()};
        const char* outNames[] = {impl_->outputName.c_str()};

        auto outputs = impl_->session->Run(Ort::RunOptions{nullptr}, inNames,
                                           &inputTensor, 1, outNames, 1);
        if (outputs.empty()) return false;

        auto info = outputs[0].GetTensorTypeAndShapeInfo();
        auto dims = info.GetShape();
        // Normalize to a 4D shape (pad leading dims with 1).
        outShape = {1, 1, 1, 1};
        const size_t rank = dims.size();
        for (size_t i = 0; i < rank && i < 4; ++i) {
            outShape[4 - rank + i] = dims[i];
        }

        const size_t count = info.GetElementCount();
        const float* data = outputs[0].GetTensorData<float>();
        output.assign(data, data + count);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace kameraa

#else  // Built without ONNX Runtime: safe no-op implementation.

namespace kameraa {

struct OnnxModel::Impl {};

OnnxModel::OnnxModel() = default;
OnnxModel::~OnnxModel() = default;

bool OnnxModel::load(const std::string&, int) { return false; }

bool OnnxModel::run(const std::vector<float>&, const std::array<int64_t, 4>&,
                    std::vector<float>&, std::array<int64_t, 4>&) {
    return false;
}

}  // namespace kameraa

#endif
