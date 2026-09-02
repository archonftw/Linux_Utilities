#include "Process.hpp"

#include <fstream>
#include <sstream>

Process::Process(int pid)
    : pid(pid),
      parentPid(-1),
      state('?') {
}

bool Process::load() {

    std::string path =
        "/proc/" + std::to_string(pid) + "/status";

    std::ifstream file(path);

    if (!file.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {

        if (line.starts_with("Name:")) {

            name = line.substr(5);

            size_t first =
                name.find_first_not_of(" \t");

            if (first != std::string::npos) {
                name = name.substr(first);
            }

        } else if (line.starts_with("State:")) {

            std::stringstream ss(line);

            std::string label;

            ss >> label >> state;

        } else if (line.starts_with("PPid:")) {

            std::stringstream ss(line);

            std::string label;

            ss >> label >> parentPid;
        }
    }

    return true;
}

bool Process::isZombie() const {

    return state == 'Z';
}