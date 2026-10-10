#pragma once

#include <atomic>
#include <string>
#include <utility>

#include "core/Interfaces.h"

namespace kameraa {

// Common enable/name handling for filters. Parameters live in derived classes
// (as atomics) so the GUI thread can update them while the worker thread reads them.
class FilterBase : public IFilter {
public:
    explicit FilterBase(std::string name) : name_(std::move(name)) {}

    std::string name() const override { return name_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }
    bool isEnabled() const override { return enabled_; }

protected:
    std::string name_;
    std::atomic<bool> enabled_{false};
};

}  // namespace kameraa
