#pragma once
#include <cstdint>

namespace os_simulation_memory {
enum class MemoryAccessType {
  READ,
  WRITE,
};

struct TraceAccess {
  uint64_t mVirtualAddress;
  MemoryAccessType mAccessType;
};

class IMemoryManager {
 public:
  virtual ~IMemoryManager() = default;
  /**
   * Returns false if page fault
   */
  virtual bool accessAddress(int processId, uint64_t virtualAddress,
                             MemoryAccessType accessType) = 0;
  virtual void handlePageFault(int processId, uint64_t virtualAddress,
                               MemoryAccessType accessType) = 0;
};
}  // namespace os_simulation_memory
