#pragma once

namespace os_simulation_process {
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

  void addTick() {
    if (mRemainingTime > 0) {
      mRemainingTime--;
    }
  }

  [[nodiscard]] int getId() const { return mId; }
  [[nodiscard]] int getPriority() const { return mPriority; }
  [[nodiscard]] int getBurstTime() const { return mBurstTime; }
  [[nodiscard]] int getArrivalTime() const { return mArrivalTime; }
  [[nodiscard]] int getRemainingTime() const { return mRemainingTime; }
  [[nodiscard]] int getStartTime() const { return mStartTime; }
  [[nodiscard]] int getCompletionTime() const { return mCompletionTime; }

  [[nodiscard]] bool isFinished() const { return mRemainingTime == 0; }
};
}  // namespace os_simulation_process