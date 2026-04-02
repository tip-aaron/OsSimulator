#pragma once

#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <thread>

namespace os_simulator {
class IProgressReporter {
 public:
  virtual ~IProgressReporter() = default;

  virtual void start(size_t totalItems) = 0;

  virtual void stop() = 0;
};

class ConsoleSpinnerReporter : public IProgressReporter {
 private:
  const char spinner[4] = {'|', '/', '-', '\\'};
  std::atomic<bool> mIsRunning{false};
  std::thread mThread;
  std::chrono::steady_clock::time_point mStartTime;
  std::function<size_t()> mGetCompletedCount;

 public:
  ~ConsoleSpinnerReporter() {
    mIsRunning = false;
    if (mThread.joinable()) {
      mThread.join();
    }
  }

  ConsoleSpinnerReporter(std::function<size_t()> lambda)
      : mGetCompletedCount(std::move(lambda)) {}

  void start(size_t totalItems) override {
    mIsRunning = true;
    mStartTime = std::chrono::steady_clock::now();
    mThread = std::thread([&, totalItems]() {
      int spinnerIdx = 0;

      while (mIsRunning) {
        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now - mStartTime)
                             .count();
        float elapsed = elapsedMs / 1000.0f;
        size_t completeCount = mGetCompletedCount();
        float percentCompleted =
            (totalItems > 0)
                ? (static_cast<float>(completeCount) / totalItems) * 100.0f
                : 0.0f;

        printf(
            "\r[%c] Simulating: %zu / %zu finished (%.1f%%) - %.2fs elapsed   ",
            spinner[spinnerIdx & 3], completeCount, totalItems,
            percentCompleted, elapsed);
        fflush(stdout);

        spinnerIdx++;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      auto now = std::chrono::steady_clock::now();
      auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - mStartTime)
                           .count();
      float elapsed = elapsedMs / 1000.0f;
      size_t completeCount = mGetCompletedCount();
      float percentCompleted =
          (totalItems > 0)
              ? (static_cast<float>(completeCount) / totalItems) * 100.0f
              : 0.0f;

      printf(
          "\r[%c] Simulating: %zu / %zu finished (%.1f%%) - %.2fs elapsed   ",
          spinner[spinnerIdx & 3], completeCount, totalItems, percentCompleted,
          elapsed);
      fflush(stdout);

      spinnerIdx++;

      printf("\r%90s\r", "");
      printf("\nFinished %zu\n at %.2fs", completeCount, elapsed);
      fflush(stdout);
    });
  }

  void stop() override {
    if (mIsRunning) {
      mIsRunning = false;

      if (mThread.joinable()) {
        mThread.join();
      }
    }
  }
};
}  // namespace os_simulator
