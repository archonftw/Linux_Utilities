#pragma once

#include <string>

class Process {
public:
    int pid;
    int parentPid;

    std::string name;
    char state;

    explicit Process(int pid);

    bool load();
    bool isZombie() const;
};