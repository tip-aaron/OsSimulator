#include <algorithm>
#include <cassert>
#include <cstdint>
#include <sim_engine.hpp>

#include "constants.hpp"
#include "events.hpp"
#include "memory.hpp"
#include "task.hpp"

using namespace std;
using namespace os_simulator;

SimulationEngine::SimulationEngine(shared_ptr<IScheduler> scheduler,
                                   shared_ptr<IMemoryManager> memoryManager,
                                   vector<unique_ptr<Task>> tasks)
    : eventQueue(make_unique<SimulationEventQueue>()),
      scheduler(scheduler),
      memoryManager(memoryManager),
      mMetrics(make_unique<OsSimulationMetrics>()) {
  this->mMetrics->initialize(tasks.size());

  for (auto &task : tasks) {
    this->eventQueue->schedule(task->getArrivalTime(),
                               SimulationEventType::TASK_ARRIVAL,
                               task->getId());
    this->scheduler->addTask(std::move(task));
  }
}

void SimulationEngine::run() {
  uint64_t mLastFairnessEvalTick = 0;
  const uint64_t FAIRNESS_INTERVAL_TICKS = 800;

  while (!eventQueue->empty()) {
    SimulationEvent event = eventQueue->extractNextEvent();

    if (event.ticks > mCurrentSimulationTicks) {
      if (mRunningTask == nullptr) {
        mMetrics->cpu.recordIdleTicks(event.ticks - mCurrentSimulationTicks);
      }

      mCurrentSimulationTicks = event.ticks;
    }

    Task *task = nullptr;

    if (event.taskId.has_value()) {
      task = scheduler->getTask(event.taskId.value());
    }

    switch (event.type) {
      case SimulationEventType::TASK_ARRIVAL:
        handleTaskArrival(task);
        break;
      case SimulationEventType::DISPATCH:
        handleTaskDispatch(task);
        break;
      case SimulationEventType::MEMORY_ACCESS:
        handleTaskMemoryAccess(task);
        break;
      case SimulationEventType::OS_ACTION:
        memoryManager->performOsAction();
        break;
    }

    if (mCurrentSimulationTicks - mLastFairnessEvalTick >=
        FAIRNESS_INTERVAL_TICKS) {
      mMetrics->cpu.evaluateIntervalFairness();

      // Update the tracker. Use division to handle massive tick jumps cleanly.
      uint64_t intervalsPassed =
          (mCurrentSimulationTicks - mLastFairnessEvalTick) /
          FAIRNESS_INTERVAL_TICKS;
      mLastFairnessEvalTick += (intervalsPassed * FAIRNESS_INTERVAL_TICKS);
    }
  }

  mMetrics->cpu.evaluateIntervalFairness();
}

void SimulationEngine::handleTaskArrival(Task *task) {
  if (task == nullptr) {
    return;
  }

  if (task->getState() == TaskState::NEW) {
    memoryManager->initializeTaskMemory(task);
    task->markReady();
    scheduler->wakeTask(task);
  } else if (task->getState() == TaskState::BLOCKED) {
    task->unblock();
    scheduler->wakeTask(task);
  }

  if (mRunningTask == nullptr) {
    eventQueue->schedule(mCurrentSimulationTicks,
                         SimulationEventType::DISPATCH);

    return;
  }

  // if there's a running task, preempt this next task.

  if (!mRunningTask->getStartTime().has_value() ||
      mCurrentSimulationTicks < mRunningTask->getStartTime().value() ||
      !scheduler->canPreempt(mRunningTask, task)) {
    return;
  }

  eventQueue->schedule(mCurrentSimulationTicks, SimulationEventType::DISPATCH,
                       mRunningTask->getId());
}

void SimulationEngine::handleTaskDispatch(Task *task) {
  if (task != nullptr && task != mRunningTask) {
    return;
  }

  Task *prevTask = mRunningTask;

  // time slice expired
  if (mRunningTask != nullptr) {
    // how long it's been ever since new task was dispatched
    uint64_t elapsed = getElapsedTicks(mRunningTask);

    executeTask(elapsed, mRunningTask);

    mRunningTask = nullptr;
  }

  // switch tasks

  Task *nextTask = scheduler->getNextTask();

  if (nextTask == nullptr || nextTask->getState() != TaskState::READY) {
    return;
  }

  mRunningTask = nextTask;
  bool isSameTask = (prevTask == nextTask);
  uint64_t startTime =
      mCurrentSimulationTicks + (isSameTask ? 0 : CONTEXT_SWITCH_COST_MS);
  uint64_t timeSlice = scheduler->calculateTimeSlice(nextTask);
  optional<MemoryPageAccess *> nextAccessOpt = nextTask->getNextPageAccess();

  if (!isSameTask) {
    mMetrics->cpu.recordContextSwitch(CONTEXT_SWITCH_COST_MS);
  }

  mMetrics->cpu.recordTaskOpportunity(nextTask->getId(), timeSlice);
  nextTask->dispatch(startTime);

  if (!nextAccessOpt.has_value()) {
    eventQueue->schedule(
        startTime + min(nextTask->getRemainingTime(), timeSlice),
        SimulationEventType::DISPATCH, nextTask->getId());

    return;
  }

  MemoryPageAccess *access = nextAccessOpt.value();
  uint64_t executedTicks = nextTask->getExecutedTicks();
  uint64_t ticksToEndAccess = 0;

  // if this page access still isn't fully executed
  if (access->relativeTick > executedTicks) {
    // remaining time before next page access
    ticksToEndAccess = access->relativeTick - executedTicks;
  }

  if (ticksToEndAccess >= timeSlice) {
    // page access ends here, means we go onto next
    // instruction
    eventQueue->schedule(startTime + timeSlice, SimulationEventType::DISPATCH,
                         nextTask->getId());
  } else {
    // execute the remaining page access instructions.
    eventQueue->schedule(startTime + ticksToEndAccess,
                         SimulationEventType::MEMORY_ACCESS, nextTask->getId());
  }
}

void SimulationEngine::handleTaskMemoryAccess(Task *task) {
  if (task == nullptr) {
    return;
  }

  if (task != mRunningTask || task->getState() != TaskState::RUNNING) {
    return;
  }

  assert(task->getStartTime().has_value() &&
         "Calling memory access but task has no start time.");

  // the time allotted for this specific memory access instruction
  uint64_t timeSlice = scheduler->calculateTimeSlice(task);
  optional<MemoryPageAccess *> nextAccessOpt = task->getNextPageAccess();

  assert(nextAccessOpt.has_value() &&
         "Called memory access but task has no more page access instruction");

  MemoryPageAccess *access = nextAccessOpt.value();
  MemoryAccessStatus status = memoryManager->accessMemory(task, *access);

  mMetrics->memory.recordAccess(task->getId(),
                                task->getNextPageAccess().value()->accessCount);

  switch (status) {
    case os_simulator::MemoryAccessStatus::PAGE_FAULT:
      handleMemoryPageFault(task);
      break;
    case os_simulator::MemoryAccessStatus::PAGE_HIT:
      handleMemoryPageHit(timeSlice, task);
      break;
  }
}

void SimulationEngine::handleMemoryPageFault(Task *task) {
  assert(task == mRunningTask && task->getStartTime().has_value());
  uint64_t elapsed = getElapsedTicks(mRunningTask);

  // 2. Advance time FIRST because it requires mState == TaskState::RUNNING
  task->advanceTime(elapsed, mCurrentSimulationTicks);
  mMetrics->cpu.recordBusyTicks(elapsed);

  if (task->isFinished()) {
    scheduler->onTaskExecuted(task, elapsed);
    mMetrics->cpu.recordTaskCompletion(task);
    scheduler->removeTask(task->getId());

    mRunningTask = nullptr;
    eventQueue->schedule(mCurrentSimulationTicks,
                         SimulationEventType::DISPATCH);
    return;
  }

  // 3. Change the state to BLOCKED *before* updating the scheduler!
  task->block();

  // 4. NOW update the scheduler. It will update the vruntime,
  // but because the state is now BLOCKED, it hits the `default:` case
  // and safely leaves the task out of the mRunQueue.
  scheduler->onTaskExecuted(task, elapsed);

  mRunningTask = nullptr;
  optional<MemoryPageAccess *> accessOpt = task->getNextPageAccess();

  assert(accessOpt.has_value());

  MemoryPageAccess *access = accessOpt.value();

  assert(access != nullptr);

  bool wasDirty = memoryManager->handlePageFault(task, *access);
  uint64_t ioDelay = BACKING_STORE_READ_LATENCY_MS +
                     (wasDirty ? BACKING_STORE_WRITE_LATENCY_MS : 0);

  mMetrics->memory.recordPageFault(task->getId());

  if (memoryManager->needsOsAction()) {
    eventQueue->schedule(
        mCurrentSimulationTicks + memoryManager->getOsActionDelay(),
        SimulationEventType::OS_ACTION);
  }

  eventQueue->schedule(mCurrentSimulationTicks + ioDelay,
                       SimulationEventType::TASK_ARRIVAL, task->getId());
  eventQueue->schedule(mCurrentSimulationTicks, SimulationEventType::DISPATCH);
}

void SimulationEngine::handleMemoryPageHit(uint64_t timeSlice, Task *task) {
  assert(task == mRunningTask);
  task->advanceMemoryIndex();

  uint64_t quantumExpirationTime = task->getStartTime().value() + timeSlice;
  optional<MemoryPageAccess *> futureAccessOpt = task->getNextPageAccess();

  // This was the last call to memory.
  // Just finish this task.
  if (!futureAccessOpt.has_value()) {
    uint64_t elapsedSoFar = getElapsedTicks(mRunningTask);
    uint64_t actualRemaining = task->getRemainingTime() > elapsedSoFar
                                   ? task->getRemainingTime() - elapsedSoFar
                                   : 0;
    uint64_t finishTime = mCurrentSimulationTicks + actualRemaining;

    if (finishTime >= quantumExpirationTime) {
      eventQueue->schedule(quantumExpirationTime, SimulationEventType::DISPATCH,
                           task->getId());
    } else {
      eventQueue->schedule(finishTime, SimulationEventType::DISPATCH,
                           task->getId());
    }

    return;
  }

  // schedule to execute the next memory accesses

  assert(mRunningTask->getStartTime().has_value());

  uint64_t elapsed = getElapsedTicks(mRunningTask);
  uint64_t executedTicks = task->getExecutedTicks() + elapsed;
  uint64_t ticksUntilNextAccess =
      futureAccessOpt.value()->relativeTick > executedTicks
          ? futureAccessOpt.value()->relativeTick - executedTicks
          : 0;

  uint64_t simTimeAtEndAccess = mCurrentSimulationTicks + ticksUntilNextAccess;

  // time slice expires/cannot be executed.
  // schedule for another dispatch
  if (simTimeAtEndAccess >= quantumExpirationTime) {
    eventQueue->schedule(quantumExpirationTime, SimulationEventType::DISPATCH,
                         task->getId());
  } else {
    // execute the memory access instruction, continuing off
    // from where the last memory access ended at.
    eventQueue->schedule(simTimeAtEndAccess, SimulationEventType::MEMORY_ACCESS,
                         task->getId());
  }
}

uint64_t SimulationEngine::getElapsedTicks(Task *task) const {
  return mCurrentSimulationTicks > task->getStartTime().value()
             ? mCurrentSimulationTicks - task->getStartTime().value()
             : 0;
}

void SimulationEngine::executeTask(uint64_t elapsed, Task *task) {
  task->advanceTime(elapsed, mCurrentSimulationTicks);
  mMetrics->cpu.recordBusyTicks(elapsed);

  scheduler->onTaskExecuted(task, elapsed);

  if (task->isFinished()) {
    mMetrics->cpu.recordTaskCompletion(task);
    scheduler->removeTask(task->getId());
  } else {
    task->preempt();
  }
}
