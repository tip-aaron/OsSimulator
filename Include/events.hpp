#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <vector>

using namespace std;

namespace os_simulator {
enum class SimulationEventType {
  TASK_ARRIVAL,
  MEMORY_ACCESS,
  DISPATCH,
  // (e.g. in MGLRU, age generations)
  OS_ACTION
};

struct SimulationEvent {
  uint64_t ticks;
  SimulationEventType type;
  optional<uint16_t> taskId;

  bool operator>(const SimulationEvent &rOther) const {
    if (ticks != rOther.ticks) {
      return ticks > rOther.ticks;
    }

    return type > rOther.type;
  }
};

inline string to_string(const SimulationEventType &eventType) {
  switch (eventType) {
    case SimulationEventType::TASK_ARRIVAL:
      return "TASK_ARRIVAL";
    case SimulationEventType::MEMORY_ACCESS:
      return "MEMORY_ACCESS";
    case SimulationEventType::DISPATCH:
      return "DISPATCH";
    case SimulationEventType::OS_ACTION:
      return "OS_ACTION";
  }
}

inline string to_string(const SimulationEvent &event) {
  return "SimulationEvent{taskId: " + std::to_string(event.taskId.value_or(0)) +
         ", eventType: " + to_string(event.type) +
         ", ticks: " + std::to_string(event.ticks) + "};";
}

class SimulationEventQueue {
 private:
  priority_queue<SimulationEvent, vector<SimulationEvent>,
                 greater<SimulationEvent>>
      mData;

 public:
  void print() {
    while (!mData.empty()) {
      cout << to_string(mData.top()) << " val" << endl;

      mData.pop();
    }
  }

  size_t size() const { return mData.size(); }

  bool empty() const { return mData.empty(); }

  void schedule(const uint64_t ticks, SimulationEventType type,
                optional<uint16_t> taskId = nullopt) {
    SimulationEvent event;

    event.ticks = ticks;
    event.type = type;
    event.taskId = taskId;

    mData.push(event);
  }

  SimulationEvent peekNextEvent() { return mData.top(); }

  SimulationEvent extractNextEvent() {
    SimulationEvent event = mData.top();

    mData.pop();

    return event;
  }
};
}  // namespace os_simulator
