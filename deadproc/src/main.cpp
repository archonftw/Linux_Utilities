#include "Process.hpp"
#include "ProcessScanner.hpp"
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include <iomanip>
#include <sstream>

volatile std::sig_atomic_t running = 1;

void handleSignal(int signal) {

    if (signal == SIGINT) {
        running = 0;
    }
}

// --------------------------------------------------
// Print human-readable process state
// --------------------------------------------------

void printState(char state) {

    switch (state) {

        case 'R':
            std::cout << "Running";
            break;

        case 'S':
            std::cout << "Sleeping";
            break;

        case 'D':
            std::cout << "Uninterruptible sleep";
            break;

        case 'T':
            std::cout << "Stopped";
            break;

        case 'Z':
            std::cout << "Zombie";
            break;

        case 'I':
            std::cout << "Idle";
            break;

        default:
            std::cout << "Unknown";
            break;
    }
}

std::string formatBytes(long bytes) {

    if (bytes < 1024) {

        return std::to_string(bytes) + " B";
    }

    double value =
        static_cast<double>(bytes) / 1024.0;

    if (value < 1024) {

        std::ostringstream output;

        output << std::fixed
               << std::setprecision(2)
               << value
               << " KB";

        return output.str();
    }

    value /= 1024.0;

    if (value < 1024) {

        std::ostringstream output;

        output << std::fixed
               << std::setprecision(2)
               << value
               << " MB";

        return output.str();
    }

    value /= 1024.0;

    std::ostringstream output;

    output << std::fixed
           << std::setprecision(2)
           << value
           << " GB";

    return output.str();
}

// --------------------------------------------------
// Inspect one process
// --------------------------------------------------

void inspectProcess(int pid) {

    Process process(pid);

    if (!process.load()) {

        std::cerr
            << "deadproc: could not read process "
            << pid
            << '\n';

        return;
    }

    std::cout << '\n';

    std::cout
        << "Process Information\n";

    std::cout
        << "════════════════════════════════════\n\n";

    std::cout
        << "PID:        "
        << process.pid
        << '\n';

    std::cout
        << "Name:       "
        << process.name
        << '\n';

    std::cout
        << "PPID:       "
        << process.parentPid
        << '\n';

    std::cout
        << "State:      "
        << process.state
        << " (";

    printState(process.state);

    std::cout
        << ")\n";

    std::cout
        << "UID:        "
        << process.uid
        << '\n';

    std::cout
        << "Threads:    "
        << process.threads
        << '\n';

    std::cout << '\n';

    std::cout
        << "Memory\n";

    std::cout
        << "────────────────────────\n";

    std::cout
        << "RSS:        "
        << process.memoryKB
        << " KB\n";

    std::cout
        << "Virtual:    "
        << process.virtualMemoryKB
        << " KB\n";

    std::cout << '\n';

    std::cout
        << "CPU\n";

    std::cout
        << "────────────────────────\n";

    std::cout
        << "User time:  "
        << process.userTime
        << " ticks\n";

    std::cout
        << "System time:"
        << ' '
        << process.systemTime
        << " ticks\n";

    std::cout << '\n';

    std::cout
        << "Context Switches\n";

    std::cout
        << "────────────────────────\n";

    std::cout
        << "Voluntary:     "
        << process.voluntaryContextSwitches
        << '\n';

    std::cout
        << "Non-voluntary: "
        << process.nonVoluntaryContextSwitches
        << '\n';

    std::cout << '\n';

    std::cout
        << "I/O\n";

    std::cout
        << "────────────────────────\n";

    std::cout
        << "Read:       "
        << formatBytes(process.readBytes)
        << '\n';

    std::cout
        << "Written:    "
        << formatBytes(process.writeBytes)
        << '\n';

    std::cout << '\n';

    std::cout
        << "Command Line\n";

    std::cout
        << "────────────────────────\n";

    if (process.commandLine.empty()) {

        std::cout
            << "(unavailable)\n";
    }
    else {

        std::cout
            << process.commandLine
            << '\n';
    }

    /*
     * Zombie diagnosis.
     */

    if (process.isZombie()) {

        std::cout << '\n';

        std::cout
            << "Diagnosis\n";

        std::cout
            << "────────────────────────\n";

        std::cout
            << "⚠ Process is a ZOMBIE.\n";

        std::cout
            << "The process has terminated but its\n"
            << "parent has not collected its exit status.\n";

        if (process.parentPid > 0) {

            Process parent(
                process.parentPid
            );

            if (parent.load()) {

                std::cout << '\n';

                std::cout
                    << "Parent Process\n";

                std::cout
                    << "PID:        "
                    << parent.pid
                    << '\n';

                std::cout
                    << "Name:       "
                    << parent.name
                    << '\n';

                std::cout
                    << "State:      "
                    << parent.state
                    << " (";

                printState(parent.state);

                std::cout
                    << ")\n";
            }
        }
    }

    std::cout << '\n';
}

// --------------------------------------------------
// Scan for zombies
// --------------------------------------------------

void scanProcesses() {

    ProcessScanner scanner;

    std::vector<Process> zombies =
        scanner.findZombies();


    std::cout << '\n';

    std::cout
        << "DEADPROC\n";

    std::cout
        << "Linux Process Diagnostics\n";

    std::cout
        << "════════════════════════════════════\n\n";

    std::cout
        << "Scanning processes...\n\n";


    for (const Process& process :
         zombies) {

        std::cout
            << "[ZOMBIE]\n";

        std::cout
            << "PID:   "
            << process.pid
            << '\n';

        std::cout
            << "Name:  "
            << process.name
            << '\n';

        std::cout
            << "PPID:  "
            << process.parentPid
            << '\n';

        std::cout
            << "────────────────────────\n";
    }


    std::cout << '\n';


    if (zombies.empty()) {

        std::cout
            << "✓ No zombie processes detected.\n";

    }
    else {

        std::cout
            << "⚠ "
            << zombies.size()
            << " zombie process";

        if (zombies.size() != 1) {
            std::cout << "es";
        }

        std::cout
            << " detected.\n";
    }

    std::cout << '\n';
}


// --------------------------------------------------
// Print process tree recursively
// --------------------------------------------------

void printTreeNode(
    const ProcessNode& node,
    const std::string& prefix,
    bool isLast,
    bool isRoot
) {

    const Process& process =
        node.process;


    /*
     * Root doesn't need a branch.
     */
    if (isRoot) {

        std::cout
            << process.name
            << " (PID "
            << process.pid
            << ")";

    }
    else {

        std::cout
            << prefix;

        if (isLast) {
            std::cout << "└── ";
        }
        else {
            std::cout << "├── ";
        }

        std::cout
            << process.name
            << " (PID "
            << process.pid
            << ")";
    }


    /*
     * State.
     */
    std::cout
        << " [";

    printState(process.state);

    std::cout
        << "]";


    /*
     * Highlight zombies.
     */
    if (process.isZombie()) {

        std::cout
            << " ⚠ ZOMBIE";
    }


    std::cout << '\n';


    /*
     * Print children.
     */
    for (size_t i = 0;
         i < node.children.size();
         ++i) {

        bool childIsLast =
            (i == node.children.size() - 1);


        std::string childPrefix;


        if (isRoot) {

            childPrefix = "";

        }
        else {

            if (isLast) {

                childPrefix =
                    prefix + "    ";

            }
            else {

                childPrefix =
                    prefix + "│   ";
            }
        }


        printTreeNode(
            *node.children[i],
            childPrefix,
            childIsLast,
            false
        );
    }
}


// --------------------------------------------------
// Display process tree
// --------------------------------------------------

void printTree(int rootPid) {

    ProcessScanner scanner;

    std::unique_ptr<ProcessNode> tree =
        scanner.buildTree(rootPid);


    if (!tree) {

        std::cerr
            << "deadproc: process "
            << rootPid
            << " not found\n";

        return;
    }


    std::cout << '\n';

    std::cout
        << "Process Tree\n";

    std::cout
        << "════════════════════════════════════\n\n";


    printTreeNode(
        *tree,
        "",
        true,
        true
    );


    std::cout << '\n';
}

void watchProcesses() {

    ProcessScanner scanner;

    std::unordered_set<int> knownZombies;

    std::cout << '\n';

    std::cout
        << "DEADPROC — Live Process Monitor\n";

    std::cout
        << "════════════════════════════════════\n\n";

    std::cout
        << "Monitoring /proc...\n";

    std::cout
        << "Press Ctrl+C to stop.\n\n";

    while (running) {

        std::vector<Process> zombies =
            scanner.findZombies();

        std::unordered_set<int> currentZombies;

        for (const Process& process :
             zombies) {

            currentZombies.insert(
                process.pid
            );

            if (!knownZombies.contains(
                    process.pid)) {

                std::cout
                    << "[NEW ZOMBIE]\n";

                std::cout
                    << "PID:   "
                    << process.pid
                    << '\n';

                std::cout
                    << "Name:  "
                    << process.name
                    << '\n';

                std::cout
                    << "PPID:  "
                    << process.parentPid
                    << '\n';

                std::cout
                    << "────────────────────────\n";
            }
        }

        for (int pid : knownZombies) {

            if (!currentZombies.contains(pid)) {

                std::cout
                    << "[RESOLVED] Zombie "
                    << pid
                    << " is no longer present.\n";
            }
        }

        knownZombies =
            std::move(currentZombies);

        std::this_thread::sleep_for(
            std::chrono::seconds(1)
        );
    }

    std::cout
        << "\nStopping monitor...\n";

    std::cout
        << "deadproc: clean shutdown.\n";
}

// --------------------------------------------------
// Usage
// --------------------------------------------------

void printStats() {

    ProcessScanner scanner;

    ProcessStats stats =
        scanner.getStats();

    std::cout << '\n';

    std::cout
        << "Process Statistics\n";

    std::cout
        << "════════════════════════════════════\n\n";

    std::cout
        << "Total processes:          "
        << stats.total
        << '\n';

    std::cout
        << "Running:                  "
        << stats.running
        << '\n';

    std::cout
        << "Sleeping:                 "
        << stats.sleeping
        << '\n';

    std::cout
        << "Uninterruptible sleep:    "
        << stats.uninterruptible
        << '\n';

    std::cout
        << "Stopped:                  "
        << stats.stopped
        << '\n';

    std::cout
        << "Zombie:                   "
        << stats.zombies
        << '\n';

    std::cout
        << "Idle:                     "
        << stats.idle
        << '\n';

    std::cout
        << "Unknown:                  "
        << stats.unknown
        << '\n';

    std::cout << '\n';
}

void printParents(int pid) {

    ProcessScanner scanner;

    std::vector<Process> chain =
        scanner.getParentChain(pid);

    if (chain.empty()) {

        std::cerr
            << "deadproc: process "
            << pid
            << " not found\n";

        return;
    }

    std::cout << '\n';

    std::cout
        << "Parent Chain\n";

    std::cout
        << "════════════════════════════════════\n\n";

    for (size_t i = 0;
         i < chain.size();
         ++i) {

        const Process& process =
            chain[i];

        std::cout
            << process.name
            << " (PID "
            << process.pid
            << ")";

        std::cout
            << " [";

        printState(process.state);

        std::cout
            << "]";

        if (process.isZombie()) {

            std::cout
                << " ⚠ ZOMBIE";
        }

        std::cout << '\n';

        if (i + 1 < chain.size()) {

            std::cout
                << "    ↑\n"
                << "    │\n";
        }
    }

    std::cout << '\n';
}

void diagnoseProcess(int pid) {

    Process process(pid);

    if (!process.load()) {

        std::cerr
            << "deadproc: process "
            << pid
            << " not found\n";

        return;
    }

    ProcessScanner scanner;

    std::vector<Process> parentChain =
        scanner.getParentChain(pid);

    std::cout << '\n';

    std::cout
        << "Process Diagnosis\n";

    std::cout
        << "════════════════════════════════════\n\n";

    std::cout
        << "Process\n"
        << "────────────────────────\n";

    std::cout
        << "PID:     "
        << process.pid
        << '\n';

    std::cout
        << "Name:    "
        << process.name
        << '\n';

    std::cout
        << "PPID:    "
        << process.parentPid
        << '\n';

    std::cout
        << "State:   "
        << process.state
        << " (";

    printState(process.state);

    std::cout
        << ")\n";


    std::cout << '\n';

    if (process.isZombie()) {

        std::cout
            << "Diagnosis\n";

        std::cout
            << "────────────────────────\n";

        std::cout
            << "⚠ Process is a ZOMBIE.\n\n";

        std::cout
            << "The process has already terminated,\n"
            << "but its parent has not collected its\n"
            << "exit status using wait()/waitpid().\n";

        if (process.parentPid > 0) {

            Process parent(
                process.parentPid
            );

            if (parent.load()) {

                std::cout << '\n';

                std::cout
                    << "Parent Process\n";

                std::cout
                    << "PID:     "
                    << parent.pid
                    << '\n';

                std::cout
                    << "Name:    "
                    << parent.name
                    << '\n';

                std::cout
                    << "State:   "
                    << parent.state
                    << " (";

                printState(parent.state);

                std::cout
                    << ")\n";

                std::cout << '\n';

                std::cout
                    << "Recommendation\n";

                std::cout
                    << "────────────────────────\n";

                std::cout
                    << "Investigate parent PID "
                    << parent.pid
                    << " ("
                    << parent.name
                    << ").\n";

                std::cout
                    << "The parent is responsible for\n"
                    << "reaping this zombie.\n";
            }
        }
    }

    else {

        std::cout
            << "Diagnosis\n";

        std::cout
            << "────────────────────────\n";

        std::cout
            << "✓ Process is not a zombie.\n";

        std::cout
            << "No zombie-specific action is required.\n";
    }


    if (!parentChain.empty()) {

        std::cout << '\n';

        std::cout
            << "Parent Chain\n";

        std::cout
            << "────────────────────────\n";

        for (size_t i = 0;
             i < parentChain.size();
             ++i) {

            const Process& current =
                parentChain[i];

            std::cout
                << current.name
                << " (PID "
                << current.pid
                << ")";

            if (current.isZombie()) {

                std::cout
                    << " ⚠ ZOMBIE";
            }

            if (i + 1 <
                parentChain.size()) {

                std::cout
                    << " → ";
            }
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

void printUsage(
    const char* program
) {

    std::cout
        << "Usage:\n\n"

        << "  "
        << program
        << "\n"
        << "      Scan for zombie processes.\n\n"

        << "  "
        << program
        << " inspect <PID>\n"
        << "      Inspect a specific process.\n\n"

        << "  "
        << program
        << " tree <PID>\n"
        << "      Display the process hierarchy.\n\n"

        << "  "
        << program
        << " watch\n"
        << "      Continuously monitor for zombies.\n"

        << "  "
        << program
        << " stats\n"
        << "      Display process statistics.\n"

        << "  "
        << program
        << " parents <PID>\n"
        << "    Show the process's parent chain.\n\n"

        << "  "
        << program
        << " diagnose <PID>\n"
        << "      Diagnose a process and its ancestry.\n\n";
}


// --------------------------------------------------
// Main
// --------------------------------------------------

int main(
    int argc,
    char* argv[]
) {

    std::signal(SIGINT, handleSignal);

    if (argc == 1) {

        scanProcesses();

        return 0;
    }

    if (
        argc == 2 &&
        std::string(argv[1]) == "watch"
    ) {

        watchProcesses();

        return 0;
    }

    if (
        argc == 2 &&
        std::string(argv[1]) == "stats"
    ) {

        printStats();

        return 0;
    }

    if (
        argc == 3 &&
        std::string(argv[1]) == "inspect"
    ) {

        try {

            int pid =
                std::stoi(argv[2]);

            if (pid <= 0) {

                std::cerr
                    << "deadproc: invalid PID\n";

                return 1;
            }

            inspectProcess(pid);

        }
        catch (...) {

            std::cerr
                << "deadproc: invalid PID\n";

            return 1;
        }

        return 0;
    }

    if (
        argc == 3 &&
        std::string(argv[1]) == "tree"
    ) {

        try {

            int pid =
                std::stoi(argv[2]);

            if (pid <= 0) {

                std::cerr
                    << "deadproc: invalid PID\n";

                return 1;
            }

            printTree(pid);

        }
        catch (...) {

            std::cerr
                << "deadproc: invalid PID\n";

            return 1;
        }

        return 0;
    }

    if (
    argc == 3 &&
    std::string(argv[1]) == "parents"
) {

    try {

        int pid =
            std::stoi(argv[2]);

        if (pid <= 0) {

            std::cerr
                << "deadproc: invalid PID\n";

            return 1;
        }

        printParents(pid);

    }
    catch (...) {

        std::cerr
            << "deadproc: invalid PID\n";

        return 1;
    }

    return 0;
}

        if (
        argc == 3 &&
        std::string(argv[1]) == "diagnose"
    ) {

        try {

            int pid =
                std::stoi(argv[2]);

            if (pid <= 0) {

                std::cerr
                    << "deadproc: invalid PID\n";

                return 1;
            }

            diagnoseProcess(pid);

        }
        catch (...) {

            std::cerr
                << "deadproc: invalid PID\n";

            return 1;
        }

        return 0;
    }

    printUsage(argv[0]);

    return 1;
}