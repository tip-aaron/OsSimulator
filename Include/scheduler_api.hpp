#pragma once
#include "process.hpp"

namespace os_simulation_scheduler {
class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void addProcess(const os_simulation_process::Process& p) = 0;
  virtual void addTick() = 0;

  [[nodiscard]] virtual bool isFinished() const = 0;
};
}  // namespace os_simulation_scheduler
