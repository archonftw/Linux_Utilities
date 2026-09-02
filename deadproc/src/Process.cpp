#include "Process.hpp"

#include <fstream>
#include <sstream>
#include <string>

Process::Process(int pid)
    : pid(pid),
      parentPid(-1),
      state('?'),
      memoryKB(0),
      virtualMemoryKB(0),
      threads(0),
      userTime(0),
      systemTime(0),
      voluntaryContextSwitches(0),
      nonVoluntaryContextSwitches(0),
      readBytes(0),
      writeBytes(0) {
}

bool Process::load() {

    std::string basePath =
        "/proc/" + std::to_string(pid);

    std::ifstream statusFile(
        basePath + "/status"
    );

    if (!statusFile.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(statusFile, line)) {

        std::stringstream ss(line);

        std::string key;

        ss >> key;

        if (key == "Name:") {

            ss >> name;
        }

        else if (key == "State:") {

            ss >> state;
        }

        else if (key == "PPid:") {

            ss >> parentPid;
        }

        else if (key == "VmSize:") {

            ss >> virtualMemoryKB;
        }

        else if (key == "VmRSS:") {

            ss >> memoryKB;
        }

        else if (key == "Threads:") {

            ss >> threads;
        }

        else if (key == "Uid:") {

            ss >> uid;
        }

        else if (key == "voluntary_ctxt_switches:") {

            ss >> voluntaryContextSwitches;
        }

        else if (key == "nonvoluntary_ctxt_switches:") {

            ss >> nonVoluntaryContextSwitches;
        }
    }

    /*
     * Read /proc/<PID>/cmdline.
     *
     * Arguments are separated by '\0', so we
     * convert them to spaces for display.
     */

    std::ifstream cmdlineFile(
        basePath + "/cmdline"
    );

    if (cmdlineFile.is_open()) {

        std::string argument;

        while (std::getline(
            cmdlineFile,
            argument,
            '\0'
        )) {

            if (!commandLine.empty()) {
                commandLine += ' ';
            }

            commandLine += argument;
        }
    }

    /*
     * Read CPU time from /proc/<PID>/stat.
     *
     * Format:
     *
     * PID (comm) state ppid ...
     *
     * Fields 14 and 15 are:
     *
     * utime
     * stime
     */

    std::ifstream statFile(
        basePath + "/stat"
    );

    if (statFile.is_open()) {

        std::string statLine;

        std::getline(
            statFile,
            statLine
        );

        std::size_t closingParen =
            statLine.rfind(')');

        if (closingParen !=
            std::string::npos) {

            std::string fields =
                statLine.substr(
                    closingParen + 2
                );

            std::stringstream ss(fields);

            char statState;

            long ppid;
            long value;

            // Field 3: state
            ss >> statState;

            // Field 4: ppid
            ss >> ppid;

            /*
             * Skip fields 5 through 13.
             */
            for (int i = 0; i < 9; ++i) {
                ss >> value;
            }

            // Field 14: utime
            ss >> userTime;

            // Field 15: stime
            ss >> systemTime;
        }
    }

    /*
     * Read process I/O statistics.
     */

    std::ifstream ioFile(
        basePath + "/io"
    );

    if (ioFile.is_open()) {

        while (std::getline(ioFile, line)) {

            std::stringstream ss(line);

            std::string key;

            long value;

            ss >> key >> value;

            if (key == "read_bytes:") {

                readBytes = value;
            }

            else if (key == "write_bytes:") {

                writeBytes = value;
            }
        }
    }

    return true;
}

bool Process::isZombie() const {

    return state == 'Z';
}