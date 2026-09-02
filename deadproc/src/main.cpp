#include "Process.hpp"
#include "ProcessScanner.hpp"

#include <iostream>
#include <string>
#include <vector>


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
        << "────────────────────────\n";

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
        << "State: "
        << process.state
        << " (";

    printState(process.state);

    std::cout << ")\n";

    if (process.isZombie()) {

        std::cout << '\n';

        std::cout
            << "Diagnosis:\n";

        std::cout
            << "This process has terminated, but its parent\n"
            << "has not collected its exit status.\n";

        std::cout
            << "The parent process has PID "
            << process.parentPid
            << ".\n";
    }

    std::cout << '\n';
}


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

    for (const Process& process : zombies) {

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

    } else {

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


void printUsage(const char* program) {

    std::cout
        << "Usage:\n\n"

        << "  "
        << program
        << "\n"
        << "      Scan for zombie processes.\n\n"

        << "  "
        << program
        << " inspect <PID>\n"
        << "      Inspect a specific process.\n";
}


int main(int argc, char* argv[]) {

    // ./deadproc
    if (argc == 1) {

        scanProcesses();

        return 0;
    }


    // ./deadproc inspect <PID>
    if (argc == 3 &&
        std::string(argv[1]) == "inspect") {

        try {

            int pid =
                std::stoi(argv[2]);

            if (pid <= 0) {

                std::cerr
                    << "deadproc: invalid PID\n";

                return 1;
            }

            inspectProcess(pid);

        } catch (...) {

            std::cerr
                << "deadproc: invalid PID\n";

            return 1;
        }

        return 0;
    }


    printUsage(argv[0]);

    return 1;
}