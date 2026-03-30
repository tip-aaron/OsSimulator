#pragma once

#include <cstdint>
#include <optional>

namespace os_simulation_discrete_event {
enum class SimulationEventType {
  PROCESS_ARRIVAL,
  DISPATCH,
  PAGE_FAULT_RESOLVED
};

struct SimulationEvent {
  uint64_t mTimestamp;
  SimulationEventType mEventType;
  std::optional<uint16_t> mPid;

  bool operator>(const SimulationEvent &other) const {
    return mTimestamp > other.mTimestamp;
  }
};
}  // namespace os_simulation_discrete_event
