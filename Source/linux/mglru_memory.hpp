#pragma once

#include <cstdint>
#include <list>
#include <memory_api.hpp>
#include <metrics.hpp>
#include <unordered_map>
#include <vector>

namespace os_simulation_memory {

struct GlobalPageId {
  int processId;
  uint64_t vpn;

  // Needed so we can use this struct as a key in std::unordered_map
  bool operator==(const GlobalPageId &other) const {
    return processId == other.processId && vpn == other.vpn;
  }
};

struct GlobalPageIdHash {
  std::size_t operator()(const GlobalPageId &k) const {
    return std::hash<int>()(k.processId) ^ (std::hash<uint64_t>()(k.vpn) << 1);
  }
};

class MglruMemoryManager : public IMemoryManager {
 public:
  static constexpr int NUM_GENERATIONS = 4;

  MglruMemoryManager();
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
    std::list<GlobalPageId>::iterator mListIterator;
  };

  std::unordered_map<GlobalPageId, PageEntry, GlobalPageIdHash> mPageTable;

  std::vector<std::list<GlobalPageId>> mGenerations;

  uint32_t mFreeFrames;

  [[nodiscard]] uint64_t getVpn(uint64_t virtualAddress) const;

  void evictPage();
};
}  // namespace os_simulation_memory
