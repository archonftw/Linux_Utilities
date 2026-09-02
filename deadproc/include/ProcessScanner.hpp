#pragma once

#include "Process.hpp"

#include <vector>

class ProcessScanner {
public:
    std::vector<Process> scan();
    std::vector<Process> findZombies();
};