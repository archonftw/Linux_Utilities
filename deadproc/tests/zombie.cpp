#include <unistd.h>

int main() {

    pid_t pid = fork();

    if (pid < 0) {
        return 1;
    }

    // Child immediately exits.
    if (pid == 0) {
        return 0;
    }

    // Parent stays alive without calling wait().
    sleep(60);

    return 0;
}