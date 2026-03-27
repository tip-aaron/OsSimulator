#pragma once

namespace os_simulation_process {
enum class ProcessState { NEW, READY, RUNNING, BLOCKED, TERMINATED };

struct Process {
 private:
  int mId;
  int mPriority;
  /**
   * Total cpu time required
   */
  int mBurstTime;
  /**
   * When it entered the system
   */
  int mArrivalTime;

  int mRemainingTime;
  /**
   * -1 means it hasn't started yet
   */
  int mStartTime{-1};
  int mCompletionTime{0};

  ProcessState mState{ProcessState::NEW};
  int mIoWaitTime{0};

 public:
  Process(int processId, int procPriority, int totalBurst, int arrival)
      : mId(processId),
        mPriority(procPriority),
        mBurstTime(totalBurst),
        mArrivalTime(arrival),
        mRemainingTime(totalBurst) {}

  void setStartTime(int time) {
    if (mStartTime == -1) {
      mStartTime = time;
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

  [[nodiscard]] int getId() const { return mId; }
  [[nodiscard]] int getPriority() const { return mPriority; }
  [[nodiscard]] int getBurstTime() const { return mBurstTime; }
  [[nodiscard]] int getArrivalTime() const { return mArrivalTime; }
  [[nodiscard]] int getRemainingTime() const { return mRemainingTime; }
  [[nodiscard]] int getStartTime() const { return mStartTime; }
  [[nodiscard]] int getCompletionTime() const { return mCompletionTime; }
  [[nodiscard]] ProcessState getState() const { return mState; }

  [[nodiscard]] bool isFinished() const {
    return mState == ProcessState::TERMINATED;
  }
};
}  // namespace os_simulation_process