#include "output/VirtualCameraFactory.h"

#include "output/CmioExtensionSink.h"
#include "output/DirectShowSink.h"
#include "output/NullVirtualCamera.h"
#include "output/V4l2LoopbackSink.h"

namespace kameraa {

std::shared_ptr<IVirtualCamera> createVirtualCamera() {
#if defined(__linux__)
    return std::make_shared<V4l2LoopbackSink>();
#elif defined(_WIN32)
    return std::make_shared<DirectShowSink>();
#elif defined(__APPLE__)
    return std::make_shared<CmioExtensionSink>();
#else
    return std::make_shared<NullVirtualCamera>(
        "none", "No virtual camera backend is available on this platform.");
#endif
}

}  // namespace kameraa
