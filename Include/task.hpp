#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace os_simulator {
enum class MemoryPageAccessType { READ, WRITE, UNKNOWN };

struct MemoryPageAccess {
  uint64_t vpn;
  MemoryPageAccessType type;
  /**
   * The specific CPU tick (relative to the start of the task)
   * when this memory access is triggered.
   */
  uint32_t relativeTick;
  /**
   * How many ticks this memory page access
   * totals to, including non-memory instruction ticks.
   */
  uint32_t totalTicks;

  uint16_t accessCount;

  /*
   * Converts a byte address to its corresponding Virtual Page Number (VPN)
   * assuming a 4KB page size.
   */
  static inline uint64_t convertByteToVpn(uint64_t byte) { return byte >> 12; }
};

inline MemoryPageAccessType from_string(string str) {
  if (str == "W" || str == "w" || str == "WRITE" || str == "write") {
    return MemoryPageAccessType::WRITE;
  }

  if (str == " R" || str == "r" || str == "READ" || str == "read") {
    return MemoryPageAccessType::READ;
  }

  return MemoryPageAccessType::UNKNOWN;
}

enum class TaskState { NEW, READY, RUNNING, BLOCKED, TERMINATED };
/**
 * @brief Represents a single unit of work (Process/Thread) in the simulation.
 * Designed to be used by both Windows (Priority-based) and Linux
 * (Fairness-based) logic.
 */
class Task {
 private:
  uint16_t mId;
  string mName;

  uint8_t mPriority;

  uint64_t mArrivalTime;
  uint64_t mBurstTime;
  uint64_t mRemainingTime;
  TaskState mState{TaskState::NEW};

  optional<uint64_t> mStartTime{std::nullopt};
  uint64_t mCompletionTime{0};
  uint32_t mPageFaults{0};

  vector<unique_ptr<MemoryPageAccess>> mPageSequence;
  size_t mNextAccessIndex = 0;

 public:
  Task(uint16_t tId, string tName, uint8_t priority, uint64_t tArrival,
       uint64_t burstTime, vector<unique_ptr<MemoryPageAccess>> pageSequence)
      : mId(tId),
        mName(tName),
        mPriority(priority),
        mArrivalTime(tArrival),
        mBurstTime(burstTime),
        mRemainingTime(burstTime),
        mPageSequence(std::move(pageSequence)) {}

  void markReady() {
    if (mState != TaskState::TERMINATED) {
      mState = TaskState::READY;
    }
  }

  void dispatch(uint64_t currentTime) {
    if (mState == TaskState::TERMINATED) {
      throw std::runtime_error("Cannot dispatch a terminated task.");
    }

    mState = TaskState::RUNNING;

    if (!mStartTime.has_value()) {
      mStartTime = currentTime;
    }
  }

  void preempt() {
    if (mState == TaskState::RUNNING) {
      mState = TaskState::READY;
    }
  }

  void block() {
    if (mState == TaskState::RUNNING) {
      mState = TaskState::BLOCKED;
      mPageFaults++;
    }
  }

  void unblock() {
    if (mState != TaskState::BLOCKED) {
      throw std::runtime_error(
          "Illegal State Transition: Cannot unblock a task that is not "
          "BLOCKED.");
    }

    mState = TaskState::READY;
  }

  void advanceMemoryIndex() {
    if (mNextAccessIndex < mPageSequence.size()) {
      mNextAccessIndex++;
    }
  }

  void advanceTime(uint64_t elapsedTicks, uint64_t currentSimulationTick) {
    if (mState != TaskState::RUNNING) [[unlikely]] {
      return;
    }

    if (elapsedTicks >= mRemainingTime) {
      mState = TaskState::TERMINATED;
      mCompletionTime = currentSimulationTick + mRemainingTime;
      mRemainingTime = 0;
    } else {
      mRemainingTime -= elapsedTicks;
    }
  }

  [[nodiscard]] uint16_t getId() const { return mId; }

  [[nodiscard]] string getName() const { return mName; }

  [[nodiscard]] uint64_t getArrivalTime() const { return mArrivalTime; }

  [[nodiscard]] uint64_t getRemainingTime() const { return mRemainingTime; }

  [[nodiscard]] uint64_t getBurstTime() const { return mBurstTime; }

  [[nodiscard]] TaskState getState() const { return mState; }

  [[nodiscard]] optional<uint64_t> getStartTime() const { return mStartTime; }

  [[nodiscard]] uint64_t getCompletionTime() const { return mCompletionTime; }

  [[nodiscard]] uint32_t getPageFaults() const { return mPageFaults; }

  [[nodiscard]] uint8_t getPriority() const { return mPriority; }

  [[nodiscard]] vector<unique_ptr<MemoryPageAccess>> &getPageSequence() {
    return mPageSequence;
  }

  [[nodiscard]] optional<MemoryPageAccess *> getNextPageAccess() const {
    if (mNextAccessIndex < mPageSequence.size()) {
      return mPageSequence[mNextAccessIndex].get();
    }

    return nullopt;
  }

  [[nodiscard]] uint64_t getExecutedTicks() const {
    return mBurstTime - mRemainingTime;
  }

  [[nodiscard]] uint64_t getTurnaroundTime() const {
    if (mCompletionTime > mArrivalTime) [[likely]] {
      return mCompletionTime - mArrivalTime;
    }

    return 0;
  }

  [[nodiscard]] uint64_t getWaitingTime() const {
    uint64_t turnaroundTime = getTurnaroundTime();

    if (turnaroundTime > mBurstTime) [[likely]] {
      return turnaroundTime - mBurstTime;
    }

    return 0;
  }

  [[nodiscard]] uint64_t getResponseTime() const {
    if (!mStartTime.has_value() || mStartTime.value() < mArrivalTime)
        [[unlikely]] {
      return 0;
    }

    return mStartTime.value() - mArrivalTime;
  }

  [[nodiscard]] bool isFinished() const {
    return mState == TaskState::TERMINATED;
  }
};
}  // namespace os_simulator
