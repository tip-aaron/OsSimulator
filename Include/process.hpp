#pragma once

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
  std::pair<uint64_t, bool> mStartTime{0, false};
  uint64_t mCompletionTime{0};

  ProcessState mState{ProcessState::NEW};
  uint64_t mIoWaitTime{0};

 public:
  Process(uint16_t processId, uint8_t procPriority, uint16_t totalBurst,
          uint64_t arrival)
      : mId(processId),
        mPriority(procPriority),
        mBurstTime(totalBurst),
        mArrivalTime(arrival),
        mRemainingTime(totalBurst) {}

  void setStartTime(int time) {
    if (mStartTime.second == false) {
      mStartTime.first = time;
      mStartTime.second = true;
    }
  }

  void setCompletionTime(int time) { mCompletionTime = time; }

  void setState(ProcessState newState) { mState = newState; }

  void block(int ticks) {
    mState = ProcessState::BLOCKED;
    mIoWaitTime = ticks;
  }

  void addTick() {
    if (mState == ProcessState::RUNNING && mRemainingTime > 0) {
      mRemainingTime--;

      if (mRemainingTime == 0) {
        mState = ProcessState::TERMINATED;
      }
    } else if (mState == ProcessState::BLOCKED && mIoWaitTime > 0) {
      mIoWaitTime--;

      if (mIoWaitTime == 0) {
        mState = ProcessState::READY;
      }
    }
  }

  [[nodiscard]] uint64_t getWaitTime() const {
    uint64_t turnaround = getTurnaroundTime();

    return (turnaround > mBurstTime) ? (turnaround - mBurstTime) : 0;
  }

  [[nodiscard]] uint64_t getTurnaroundTime() const {
    return (mCompletionTime > mArrivalTime) ? (mCompletionTime - mArrivalTime) : 0;
  }

  [[nodiscard]] uint64_t getResponseTime() const {
    if (!mStartTime.second || mStartTime.first < mArrivalTime) {
      return 0;
    }

    return mStartTime.first - mArrivalTime;
  }

  [[nodiscard]] uint16_t getId() const { return mId; }

  [[nodiscard]] uint8_t getPriority() const { return mPriority; }

  [[nodiscard]] uint16_t getBurstTime() const { return mBurstTime; }

  [[nodiscard]] uint64_t getArrivalTime() const { return mArrivalTime; }

  [[nodiscard]] uint64_t getRemainingTime() const { return mRemainingTime; }

  [[nodiscard]] uint64_t getStartTime() const { return mStartTime.first; }

  [[nodiscard]] uint64_t getCompletionTime() const { return mCompletionTime; }

  [[nodiscard]] ProcessState getState() const { return mState; }

  [[nodiscard]] bool isFinished() const {
    return mState == ProcessState::TERMINATED;
  }
};
}  // namespace os_simulation_process