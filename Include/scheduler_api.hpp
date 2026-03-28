#pragma once
#include "process.hpp"

namespace os_simulation_scheduler {
class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void addProcess(const os_simulation_process::Process &rProcess) = 0;
  virtual void addTick() = 0;

  virtual os_simulation_process::Process *getNextProcessToRun() = 0;
  virtual void executeProcess(os_simulation_process::Process *pProcess) = 0;

  [[nodiscard]] virtual bool isFinished() const = 0;
};
}  // namespace os_simulation_scheduler
