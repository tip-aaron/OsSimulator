#pragma once
#include "process.hpp"


class IScheduler {
public:
    virtual ~IScheduler() = default;

    virtual void addProcess(const Process& p) = 0;

    virtual void tick() = 0;

    [[nodiscard]] virtual bool isFinished() const = 0;
};

