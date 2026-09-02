#pragma once

#include "Process.hpp"

#include <memory>
#include <vector>

struct ProcessNode {

    Process process;

    std::vector<std::unique_ptr<ProcessNode>> children;

    explicit ProcessNode(const Process& process)
        : process(process) {
    }
};

struct ProcessStats {

    int total = 0;

    int running = 0;
    int sleeping = 0;
    int uninterruptible = 0;
    int stopped = 0;
    int zombies = 0;
    int idle = 0;
    int unknown = 0;
};

class ProcessScanner {
public:

    std::vector<Process> scan();

    std::vector<Process> findZombies();

    ProcessStats getStats();

    std::vector<Process> getParentChain(int pid);

    std::unique_ptr<ProcessNode>
    buildTree(int rootPid);
};