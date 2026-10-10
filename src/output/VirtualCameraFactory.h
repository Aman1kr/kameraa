#pragma once

#include <memory>

#include "core/Interfaces.h"

namespace kameraa {

// Returns the virtual-camera backend appropriate for the current platform.
// Phase 1 implements Linux (v4l2loopback); Windows/macOS return a guidance stub
// until their native backends land (Phase 4 / Phase 5).
std::shared_ptr<IVirtualCamera> createVirtualCamera();

}  // namespace kameraa
