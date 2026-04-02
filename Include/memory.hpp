#pragma once

#include <string>

#include "task.hpp"

using namespace std;

namespace os_simulator {
enum class MemoryAccessStatus {
  PAGE_HIT,   // The page was already in physical RAM. Fast access!
  PAGE_FAULT  // The page was not in RAM. Engine must fetch it from disk.
};

class IMemoryManager {
 public:
  virtual ~IMemoryManager() = default;

  /**
   * @brief Provides a human-readable name of the memory manager setup.
   * Example: "Linux LRU Page Replacement" or "Windows Working Set Manager".
   */
  [[nodiscard]] virtual string getAlgorithmName() const = 0;

  virtual void initializeTaskMemory(Task *pTask) = 0;

  [[nodiscard]] virtual MemoryAccessStatus accessMemory(
      Task *task, const MemoryPageAccess &rMemoryPageAccess) = 0;

  virtual bool handlePageFault(Task *pTask,
                               const MemoryPageAccess &rMemoryPageAccess) = 0;

  virtual bool needsOsAction() = 0;

  virtual uint64_t getOsActionDelay() = 0;

  /**
   * @brief Called when a task terminates to clear its local memory tracking.
   * @param task The terminating task.
   */
  virtual void cleanupTaskMemory(Task *task) = 0;

  virtual void performOsAction() = 0;
};
}  // namespace os_simulator
