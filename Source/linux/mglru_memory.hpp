#pragma once

#include <cstdint>
#include <list>
#include <memory_api.hpp>
#include <metrics.hpp>
#include <unordered_map>
#include <vector>

namespace os_simulation_memory {

class MglruMemoryManager : public IMemoryManager {
 public:
  static constexpr int NUM_GENERATIONS = 4;

  /**
   * Explicit to prevent implicit conversions,
   * ensuring the caller must explicitly provide a MemoryMetrics
   * object when creating an instance of MglruMemoryManager,
   * making the code clearer and preventing unintended conversions.
   */
  explicit MglruMemoryManager(
      os_simulation_metrics::MemoryMetrics& memoryMetrics);
  ~MglruMemoryManager() override = default;

  /**
   * Simulates a memory access.
   * Returns true if page is in memory, false if it causes a page fault.
   */
  bool accessAddress(int processId, uint64_t virtualAddress,
                     MemoryAccessType accessType) override;

  /**
   * Handles bringing the page into memory, potentially evicting an older page.
   */
  void handlePageFault(int processId, uint64_t virtualAddress,
                       MemoryAccessType accessType) override;

  /**
   * MGLRU specific: Simulates the kernel's background aging sweep.
   * The OS simulator should call this periodically (e.g., every N clock ticks).
   * It promotes referenced pages to Gen 0 and demotes unreferenced pages
   * towards the oldest generation.
   */
  void ageGenerations();

 private:
  struct PageEntry {
    int mGeneration;
    bool mReferenced;
    bool mDirty;
    std::list<uint64_t>::iterator mListIterator;
  };

  std::unordered_map<uint64_t, PageEntry> mPageTable;

  std::vector<std::list<uint64_t>> mGenerations;

  uint32_t mFreeFrames;

  os_simulation_metrics::MemoryMetrics& mMemoryMetrics;

  [[nodiscard]] uint64_t getVpn(uint64_t virtualAddress) const;

  void evictPage();
};
}  // namespace os_simulation_memory
