#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>

namespace os_simulation_process {

enum class ProcessState { NEW, READY, RUNNING, BLOCKED, TERMINATED };

struct Process {
 private:
  uint16_t mId;

  uint8_t mPriority;

  /**
   * Total cpu time required
   */
  uint16_t mBurstTime;
  /**
   * When it entered the system
   */
  uint64_t mArrivalTime;
  uint64_t mRemainingTime;
  /*
   * A pair, indicating if it started or not
   */
  std::optional<uint64_t> mStartTime{std::nullopt};
  uint64_t mCompletionTime{0};

  ProcessState mState{ProcessState::NEW};

 public:
  Process(uint16_t processId, uint8_t procPriority, uint16_t totalBurst,
          uint64_t arrival)
      : mId(processId),
        mPriority(procPriority),
        mBurstTime(totalBurst),
        mArrivalTime(arrival),
        mRemainingTime(totalBurst) {}

  /**
   * @brief Transitions the process to the READY state.
   * Valid from NEW, BLOCKED, or RUNNING (via preemption).
   * @throws std::runtime_error if attempting to ready a TERMINATED process.
   */
  void markReady() {
    if (mState == ProcessState::TERMINATED) {
      throw std::runtime_error(
          "Illegal State Transition: Cannot ready a TERMINATED process.");
    }

    mState = ProcessState::READY;
  }

  /**
   * @brief Transitions the process to the RUNNING state.
   * Automatically records the start time if this is the first dispatch.
   * @param currentTime The current simulation time (used to set start time).
   * @throws std::runtime_error if process is not NEW or READY.
   */
  void dispatch(uint64_t currentTime) {
    if (mState != ProcessState::READY && mState != ProcessState::NEW) {
      throw std::runtime_error(
          "Illegal State Transition: Process must be READY or NEW to be "
          "dispatched.");
    }

    mState = ProcessState::RUNNING;

    if (!mStartTime.has_value()) {
      mStartTime = currentTime;
    }
  }

  /**
   * @brief Preempts a RUNNING process, moving it back to READY.
   */
  void preempt() {
    if (mState == ProcessState::RUNNING) {
      mState = ProcessState::READY;
    }
  }

  /**
   * @brief Transitions a RUNNING process to BLOCKED (e.g., waiting for I/O or
   * page fault).
   * @throws std::runtime_error if the process is not RUNNING.
   */
  void block() {
    if (mState != ProcessState::RUNNING) {
      throw std::runtime_error(
          "Illegal State Transition: Only RUNNING processes can be blocked.");
    }

    mState = ProcessState::BLOCKED;
  }

  /**
   * @brief Advances the process execution by a given number of ticks.
   * If the process completes its burst time, it automatically transitions to
   * TERMINATED and records its completion time.
   * @param ticksExecuted The amount of time the process spent running on the
   * CPU.
   * @param currentTime The current simulation time (used if the process
   * terminates).
   * @return true if the process finished during this burst, false otherwise.
   * @throws std::runtime_error if the process is not RUNNING.
   */
  bool executeTicks(uint64_t ticksExecuted, uint64_t currentTime) {
    if (mState != ProcessState::RUNNING) {
      throw std::runtime_error(
          "Illegal Operation: Cannot execute a process that is not RUNNING.");
    }

    if (mRemainingTime > ticksExecuted) {
      mRemainingTime -= ticksExecuted;
      return false;
    } else {
      mRemainingTime = 0;
      mState = ProcessState::TERMINATED;
      mCompletionTime = currentTime;
      return true;
    }
  }

  [[nodiscard]] uint64_t getWaitTime() const {
    uint64_t turnaround = getTurnaroundTime();

    return (turnaround > mBurstTime) ? (turnaround - mBurstTime) : 0;
  }

  [[nodiscard]] uint64_t getTurnaroundTime() const {
    return (mCompletionTime > mArrivalTime) ? (mCompletionTime - mArrivalTime)
                                            : 0;
  }

  [[nodiscard]] uint64_t getResponseTime() const {
    if (!mStartTime.has_value() || mStartTime.value() < mArrivalTime) {
      return 0;
    }

    return mStartTime.value() - mArrivalTime;
  }

  [[nodiscard]] uint16_t getId() const { return mId; }

  [[nodiscard]] uint8_t getPriority() const { return mPriority; }

  [[nodiscard]] uint16_t getBurstTime() const { return mBurstTime; }

  [[nodiscard]] uint64_t getArrivalTime() const { return mArrivalTime; }

  [[nodiscard]] uint64_t getRemainingTime() const { return mRemainingTime; }

  [[nodiscard]] uint64_t getStartTime() const { return mStartTime.value_or(0); }

  [[nodiscard]] uint64_t getCompletionTime() const { return mCompletionTime; }

  [[nodiscard]] ProcessState getState() const { return mState; }

  [[nodiscard]] bool isFinished() const {
    return mState == ProcessState::TERMINATED;
  }
};
}  // namespace os_simulation_process