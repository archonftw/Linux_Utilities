#include "ProcessScanner.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;


static bool isPidDirectory(const std::string& name) {

    if (name.empty()) {
        return false;
    }

    return std::all_of(
        name.begin(),
        name.end(),
        [](unsigned char c) {
            return std::isdigit(c);
        }
    );
}


std::vector<Process> ProcessScanner::scan() {

    std::vector<Process> processes;

    try {

        for (const auto& entry :
             fs::directory_iterator("/proc")) {

            std::string name =
                entry.path()
                    .filename()
                    .string();

            if (!isPidDirectory(name)) {
                continue;
            }

            int pid;

            try {

                pid = std::stoi(name);

            } catch (...) {

                continue;
            }

            Process process(pid);

            // A process can disappear between
            // seeing its directory and reading it.
            if (!process.load()) {
                continue;
            }

            processes.push_back(process);
        }

    } catch (const fs::filesystem_error&) {

        return processes;
    }

    return processes;
}


std::vector<Process> ProcessScanner::findZombies() {

    std::vector<Process> zombies;

    std::vector<Process> processes = scan();

    for (const Process& process : processes) {

        if (process.isZombie()) {
            zombies.push_back(process);
        }
    }

    return zombies;
}