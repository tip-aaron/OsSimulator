#pragma once

struct Process {
private:
    int pId;
    int priority;
    /**
     * Total cpu time required
     */
    int burstTime;
    /**
     * When it entered the system
     */
    int arrivalTime;

    int remainingTime;
    /**
     * -1 means it hasn't started yet
     */
    int startTime{-1};  
    int completionTime{0};

public:
    Process(int processId, int procPriority, int totalBurst, int arrival)
        : pId(processId), priority(procPriority), burstTime(totalBurst), 
          arrivalTime(arrival), remainingTime(totalBurst) {}

    void setStartTime(int time) {
        if (startTime == -1) {
            startTime = time;
        }
    }

    void setCompletionTime(int time) {
        completionTime = time;
    }

    void tick() {
        if (remainingTime > 0) {
            remainingTime--;
        }
    }

    [[nodiscard]] int getId() const             { return pId; }
    [[nodiscard]] int getPriority() const       { return priority; }
    [[nodiscard]] int getBurstTime() const      { return burstTime; }
    [[nodiscard]] int getArrivalTime() const    { return arrivalTime; }
    [[nodiscard]] int getRemainingTime() const  { return remainingTime; }
    [[nodiscard]] int getStartTime() const      { return startTime; }
    [[nodiscard]] int getCompletionTime() const { return completionTime; }

    [[nodiscard]] bool isFinished() const { 
        return remainingTime == 0; 
    }
};

