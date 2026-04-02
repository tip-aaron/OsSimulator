#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "constants.hpp"
#include "rbtree.hpp"
#include "scheduler.hpp"

namespace os_simulator {
// === FROM LINUX KERNEL CODE ===
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
// === END LINUX KERNEL CODE ===

/**
 * @brief Converts a 1-10 priority scale to Linux's -20 to 19 nice value scale.
 * * This function uses linear interpolation to map a custom priority range
 * [1, 10] to the standard Linux Completely Fair Scheduler (CFS) nice
 * value range [-20, 19].
 * * The mapping assumes:
 * - Priority 1 (highest) maps to Nice -20 (maximum CPU share)
 * - Priority 10 (lowest) maps to Nice 19 (minimum CPU share)
 * * The mathematical formula used is: N = -20 + ((P - 1) * 39) / 9
 * * @param priority The raw priority value, expected to be in the range [1,
 * 10]. Values outside this range are safely clamped.
 * @return int The corresponding Linux nice value in the range [-20, 19].
 */
static int __convPriorityToNice(uint8_t priority) {
  return -20 + (clamp(priority, MIN_PRIORITY, MAX_PRIORITY) - 1) * 39 / 9;
}

/**
 * @brief Wrapper node for Linux CFS.
 * Holds the simulation task alongside Linux-specific metadata for the Red-Black
 * Tree.
 */
struct LinuxTaskNode {
  unique_ptr<Task> task;

  uint32_t weight;
  uint64_t inverseWeight;

  uint64_t vruntime{0};
  bool inRunQueue{false};

  explicit LinuxTaskNode(unique_ptr<Task> t) : task(std::move(t)) {
    size_t arrayIndex =
        clamp(__convPriorityToNice(this->task->getPriority()) + 20, 0, 39);

    weight = SCHED_PRIO_TO_WEIGHT[arrayIndex];
    inverseWeight = SCHED_PRIO_TO_WMULT[arrayIndex];
  }
};

struct TaskHandle {
  shared_ptr<os_simulator::LinuxTaskNode> pTask;

  bool operator<(const TaskHandle &other) const {
    if (this->pTask->vruntime != other.pTask->vruntime) {
      return this->pTask->vruntime < other.pTask->vruntime;
    }

    return this->pTask->task->getId() < other.pTask->task->getId();
  }

  bool operator==(const TaskHandle &other) const {
    return this->pTask->task->getId() == other.pTask->task->getId();
  }
};

class LinuxScheduler : public IScheduler {
 public:
  ~LinuxScheduler() override = default;

  [[nodiscard]] string getAlgorithmName() const override {
    return "Linux Completely Fair Scheduler (CFS)";
  }

  void addTask(unique_ptr<Task> task) override;

  void wakeTask(Task *task) override;

  [[nodiscard]] Task *getNextTask() override;

  [[nodiscard]] uint64_t calculateTimeSlice(Task *task) const override;

  void onTaskExecuted(Task *task, uint64_t elapsedTicks) override;

  void removeTask(uint16_t taskId) override;

  [[nodiscard]] bool canPreempt(Task *pRunningTask,
                                Task *pNewTask) const override;

  Task *getTask(uint16_t taskId) override;

  [[nodiscard]] bool runQueueEmpty();

  [[nodiscard]] bool empty() override { return runQueueEmpty(); }

 private:
  uint64_t mTotalRunqueueWeight{0};

  uint64_t mMinVruntime{0};

  unordered_map<uint16_t, shared_ptr<LinuxTaskNode>> mNodeMap;
  RbTree<TaskHandle> mRunQueue;

  [[nodiscard]] shared_ptr<LinuxTaskNode> findNode(const uint16_t taskId) const;

  [[nodiscard]] uint64_t calcDelta(uint64_t deltaExec, uint32_t weight,
                                   uint64_t inverseWeight);
};
}  // namespace os_simulator
