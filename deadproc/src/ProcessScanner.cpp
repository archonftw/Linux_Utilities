#include "ProcessScanner.hpp"
#include <functional>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;


// Check whether a directory name is a PID
static bool isPidDirectory(
    const std::string& name
) {

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


// Scan all processes
std::vector<Process>
ProcessScanner::scan() {

    std::vector<Process> processes;

    try {

        for (const auto& entry :
             fs::directory_iterator("/proc")) {

            std::string name =
                entry.path()
                    .filename()
                    .string();

            // Ignore non-PID directories.
            if (!isPidDirectory(name)) {
                continue;
            }

            int pid;

            try {

                pid = std::stoi(name);

            }
            catch (...) {

                continue;
            }

            Process process(pid);

            /*
             * Processes can disappear while
             * we're scanning /proc.
             */
            if (!process.load()) {
                continue;
            }

            processes.push_back(process);
        }

    }
    catch (const fs::filesystem_error&) {

        return processes;
    }

    return processes;
}


// Find zombies
std::vector<Process>
ProcessScanner::findZombies() {

    std::vector<Process> zombies;

    std::vector<Process> processes =
        scan();

    for (const Process& process :
         processes) {

        if (process.isZombie()) {

            zombies.push_back(process);
        }
    }

    return zombies;
}

ProcessStats
ProcessScanner::getStats() {

    ProcessStats stats;

    std::vector<Process> processes =
        scan();

    for (const Process& process :
         processes) {

        stats.total++;

        switch (process.state) {

            case 'R':
                stats.running++;
                break;

            case 'S':
                stats.sleeping++;
                break;

            case 'D':
                stats.uninterruptible++;
                break;

            case 'T':
                stats.stopped++;
                break;

            case 'Z':
                stats.zombies++;
                break;

            case 'I':
                stats.idle++;
                break;

            default:
                stats.unknown++;
                break;
        }
    }

    return stats;
}

std::vector<Process>
ProcessScanner::getParentChain(int pid) {

    std::vector<Process> chain;

    std::vector<Process> processes =
        scan();

    std::unordered_map<int, Process>
        processesByPid;

    for (const Process& process :
         processes) {

        processesByPid.emplace(
            process.pid,
            process
        );
    }

    auto current =
        processesByPid.find(pid);

    if (current == processesByPid.end()) {
        return chain;
    }

    while (current != processesByPid.end()) {

        const Process& process =
            current->second;

        chain.push_back(process);

        // PID 1 is normally the top
        // of the userspace process tree.
        if (process.pid == 1) {
            break;
        }

        int parentPid =
            process.parentPid;

        // Protect against malformed/cyclic
        // process relationships.
        if (parentPid <= 0 ||
            parentPid == process.pid) {
            break;
        }

        current =
            processesByPid.find(parentPid);
    }

    return chain;
}

// Build process tree
std::unique_ptr<ProcessNode>
ProcessScanner::buildTree(int rootPid) {
    std::vector<Process> processes =
        scan();
    std::unordered_map<int, Process>
        processesByPid;

    for (const Process& process :
         processes) {

        processesByPid.emplace(
            process.pid,
            process
        );
    }

    auto rootIt =
        processesByPid.find(rootPid);

    if (rootIt ==
        processesByPid.end()) {

        return nullptr;
    }
    std::unordered_map<int, std::vector<int>>
        children;

    for (const Process& process :
         processes) {

        children[process.parentPid]
            .push_back(process.pid);
    }
    std::function<std::unique_ptr<ProcessNode>(int)>
        buildNode;


    buildNode =
        [&](int pid)
        -> std::unique_ptr<ProcessNode> {

            auto processIt =
                processesByPid.find(pid);

            if (processIt ==
                processesByPid.end()) {

                return nullptr;
            }
            auto node =
                std::make_unique<ProcessNode>(
                    processIt->second
                );

            auto childrenIt =
                children.find(pid);

            if (childrenIt ==
                children.end()) {

                return node;
            }


            for (int childPid :
                 childrenIt->second) {

                auto child =
                    buildNode(childPid);

                if (child) {

                    node->children.push_back(
                        std::move(child)
                    );
                }
            }

            return node;
        };


    return buildNode(rootPid);
}