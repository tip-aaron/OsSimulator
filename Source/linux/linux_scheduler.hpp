#pragma once
#include <cstdint>
#include <vector>

#include "scheduler_api.hpp"

// === FROM LINUX KERNEL CODE ===
namespace os_simulation_linux_scheduler_math {
constexpr uint32_t SCHED_FIXEDPOINT_SHIFT = 10;
constexpr uint32_t SCHED_FIXEDPOINT_SCALE = 1U << SCHED_FIXEDPOINT_SHIFT;

constexpr uint32_t NICE_0_LOAD_SHIFT = SCHED_FIXEDPOINT_SHIFT;
constexpr uint32_t NICE_0_LOAD = 1U << NICE_0_LOAD_SHIFT;

/*
 * delta_exec * weight / lw.weight
 * OR
 * (delta_exec * (weight * lw->inv_weight)) >> WMULT_SHIFT
 *
 * Weight multiplier/Inverse
 */
constexpr uint32_t WMULT_SHIFT = 32;

/*
 * Default tasks should be treated as a thread within one group.
 */
constexpr uint32_t SCHED_PRIO_TO_WEIGHT[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */ 9548,  7620,  6100,  4904,  3906,
    /* -5 */ 3121,   2501,  1991,  1586,  1277,
    /* 0 */ 1024,    820,   655,   526,   423,
    /* 5 */ 335,     272,   215,   172,   137,
    /* 10 */ 110,    87,    70,    56,    45,
    /* 15 */ 36,     29,    23,    18,    15,
};

/*
 * Inverse weight, with shift of 32.
 */
constexpr uint32_t SCHED_PRIO_TO_WMULT[40] = {
    /* -20 */ 48388,    59856,     76040,     92818,     118348,
    /* -15 */ 147320,   184698,    229616,    287308,    360437,
    /* -10 */ 449829,   563644,    704093,    875809,    1099582,
    /* -5 */ 1376151,   1717300,   2157191,   2708050,   3363326,
    /* 0 */ 4194304,    5237765,   6557202,   8165337,   10153587,
    /* 5 */ 12820798,   15790321,  19976592,  24970740,  31350126,
    /* 10 */ 39045157,  49367440,  61356676,  76695844,  95443717,
    /* 15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};
}  // namespace os_simulation_linux_scheduler_math
// === END LINUX KERNEL CODE ===

namespace os_simulation_linux_scheduler {

struct CfsNode {
  os_simulation_process::Process mProcess;

  uint64_t mVruntime{0};
  uint32_t mWeight{0};
  uint32_t mInverseWeight{0};

  // explicit to avoid implicit conversions of Process.
  explicit CfsNode(const os_simulation_process::Process &rProcess);
};

enum class RbColor { RED, BLACK };

/**
 * @brief Internal node structure for the Red-Black Tree
 */
struct RbNode {
  CfsNode *pCfsNode{nullptr};
  uint64_t mVruntime{0};

  RbNode *pParent{nullptr};
  RbNode *pLeft{nullptr};
  RbNode *pRight{nullptr};

  RbColor mColor{RbColor::BLACK};
};

/**
 * @brief A custom Red-Black Tree optimized for the Linux CFS simulation.
 * Sorts nodes based on their virtual runtime (vruntime).
 */
class RedBlackTree {
 private:
  RbNode *mRoot;
  RbNode
      *mNil;  // Sentinel node used to represent leaves and simplify edge cases

  void rotateLeft(RbNode *pNode);
  void rotateRight(RbNode *pNode);

  void insertFixup(RbNode *pNode);
  void removeFixup(RbNode *pNode);

  void transplant(RbNode *pTarget, RbNode *pReplacement);
  RbNode *getMinimum(RbNode *pNode) const;

  void destroyTree(RbNode *pNode);

 public:
  RedBlackTree();
  ~RedBlackTree();

  // Disable copy/move to prevent accidental deep copy overheads
  RedBlackTree(const RedBlackTree &) = delete;
  RedBlackTree &operator=(const RedBlackTree &) = delete;

  /**
   * @brief Inserts a CfsNode into the tree based on its vruntime.
   */
  void insert(CfsNode *pCfsNode, uint64_t vruntime);

  /**
   * @brief Extracts and removes the node with the lowest vruntime.
   * @return CfsNode* The node with the minimum vruntime, or nullptr if empty.
   */
  CfsNode *extractMin();
};

class LinuxCfsScheduler : public os_simulation_scheduler::IScheduler {
 private:
  // The DES runqueue utilizing the custom Red-Black tree
  RedBlackTree mRunQueue;

  // The backing store for all process instances to prevent memory fragmentation
  std::vector<CfsNode> mAllNodes;

  static uint64_t calcDelta(uint64_t deltaExec, uint64_t weight,
                            uint64_t inverseWeight);

 public:
  void addProcess(const os_simulation_process::Process &rProcess) override;
  void readyProcess(uint16_t processId) override;

  uint64_t getPreemptionDelay(
      os_simulation_process::Process *pProcess) override;
  os_simulation_process::Process *getProcess(uint16_t processId) override;
  os_simulation_process::Process *getNextProcessToRun() override;

  void updateProcessExecution(os_simulation_process::Process *pProcess,
                              uint64_t executedTicks,
                              uint64_t currentTime) override;

  void preemptProcess(os_simulation_process::Process *pProcess) override;
  [[nodiscard]] bool isFinished() const override;

  [[nodiscard]] const os_simulation_process::Process &getProcessConst(
      uint16_t processId) const;
};
}  // namespace os_simulation_linux_scheduler