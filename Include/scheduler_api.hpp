#pragma once
#include "process.hpp"

namespace os_simulation_scheduler {
class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void addProcess(const os_simulation_process::Process &rProcess) = 0;

  virtual void readyProcess(uint16_t processId) = 0;

  virtual uint64_t getPreemptionDelay(
      os_simulation_process::Process *pProcess) = 0;

  virtual os_simulation_process::Process *getProcess(uint16_t processId) = 0;
  virtual os_simulation_process::Process *getNextProcessToRun() = 0;

  /**
   * @brief Updates process states and internal scheduler math after a burst
   * execution.
   */
  virtual void updateProcessExecution(os_simulation_process::Process *pProcess,
                                      uint64_t executedTicks,
                                      uint64_t currentTime) = 0;

  /**
   * @brief Preempts the current process and puts it back in the runqueue.
   */
  virtual void preemptProcess(os_simulation_process::Process *pProcess) = 0;

  [[nodiscard]] virtual bool isFinished() const = 0;
};
}  // namespace os_simulation_scheduler
