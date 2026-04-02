#pragma once

#include <list>
#include <set>
#include <unordered_map>
#include <utility>

#include "constants.hpp"
#include "memory.hpp"
#include "task.hpp"

using namespace std;

namespace os_simulator {
struct PageNode {
  uint16_t taskId;
  uint64_t vpn;
  bool isDirty{false};
};

using GlobalPageRef = std::pair<uint16_t, uint64_t>;

struct LocalMglruState {
  list<PageNode> generations[LINUX_MGLRU_MAX_GENERATIONS];

  // Maps VPN -> {Generation Index, Iterator to node}
  struct PageLocation {
    uint8_t generationIndex;
    list<PageNode>::iterator it;
    std::list<GlobalPageRef>::iterator git;
  };

  unordered_map<uint64_t, PageLocation> pageMap;

  uint32_t currentFramesUsed{0};
};

class LinuxMemoryManager : public IMemoryManager {
 public:
  explicit LinuxMemoryManager()
      : mGlobalFramesUsed(0), mGlobalFrameLimit(PHYSICAL_FRAME_COUNT) {}

  string getAlgorithmName() const override { return "Linux  Local MGLRU"; }

  void initializeTaskMemory(Task *pTask) override;

  [[nodiscard]] MemoryAccessStatus accessMemory(
      Task *pTask, const MemoryPageAccess &rMemoryPageAccess) override;

  bool handlePageFault(Task *pTask,
                       const MemoryPageAccess &rMemoryPageAccess) override;

  void cleanupTaskMemory(Task *task) override;
  bool needsOsAction() override;

  void performOsAction() override;

  uint64_t getOsActionDelay() override {
    return LINUX_MGLRU_AGING_SWEEP_INTERVAL_MS;
  }

 private:
  set<uint16_t> mActiveTasks;
  uint32_t mGlobalFramesUsed;
  uint32_t mGlobalFrameLimit;
  unordered_map<uint16_t, LocalMglruState> mTaskMemoryStates;
  list<pair<uint16_t, uint64_t>>
      mGlobalGenerations[LINUX_MGLRU_MAX_GENERATIONS];

  void promotePage(LocalMglruState &rState, uint64_t vpn);

  void ageGenerations(LocalMglruState &rState);

  bool evictOldest();
};
}  // namespace os_simulator