#pragma once
#include "scheduler_api.hpp"
#include <vector>

class LinuxCfsScheduler : public IScheduler {
private:
    struct CfsNode {
        Process process;
        double vruntime{ 0.0 };
        double weight;

        explicit CfsNode(const Process& p);
    };

    std::vector<CfsNode> nodes;
    int currentTime{ 0 };

public:
    void addProcess(const Process& p) override;
    void tick() override;
    [[nodiscard]] bool isFinished() const override;
};