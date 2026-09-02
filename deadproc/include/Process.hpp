#pragma once

#include <string>

class Process {
public:
    int pid;
    int parentPid;

    std::string name;
    std::string commandLine;

    char state;

    long memoryKB;
    long virtualMemoryKB;

    int threads;

    long userTime;
    long systemTime;

    long voluntaryContextSwitches;
    long nonVoluntaryContextSwitches;

    long readBytes;
    long writeBytes;

    std::string uid;

    explicit Process(int pid);

    bool load();
    bool isZombie() const;
};