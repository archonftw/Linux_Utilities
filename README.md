# deadproc

**deadproc** is a Linux process diagnostics and monitoring utility written in **C++20**.

It interacts directly with the Linux `/proc` filesystem to inspect and monitor processes without relying on external utilities such as `ps` or `top`.

The goal of the project is not just to provide process information, but to explore how Linux exposes process state and how operating-system concepts can be implemented from userspace.

---

## Features

### 🔍 Process inspection

Inspect detailed information about a process:

```bash
./deadproc inspect <PID>
```

Displays information such as:

* PID
* Parent PID
* Process name
* Process state
* UID
* Thread count
* Resident memory
* Virtual memory
* CPU time
* Context switches
* I/O statistics
* Command line

---

### 🧟 Zombie detection

Run:

```bash
./deadproc
```

deadproc scans `/proc` and identifies processes whose state is `Z`.

Example:

```text
[ZOMBIE] PID=4217 Name=test PPID=4182
1 zombie process detected.
```

A zombie process is a process that has already terminated but whose parent has not yet collected its exit status.

The basic lifecycle is:

```text
fork()
   │
   ▼
Child executes
   │
   ▼
exit()
   │
   ▼
Zombie
   │
   │ parent calls wait()/waitpid()
   ▼
Process entry removed
```

---

## 🩺 Process diagnosis

Diagnose a process with:

```bash
./deadproc diagnose <PID>
```

For zombie processes, deadproc also investigates the parent process.

Example:

```text
Process Diagnosis
════════════════════════════════════

PID: 4217
Name: worker
PPID: 4182
State: Z (Zombie)

Diagnosis
────────────────────────
⚠ Process is a ZOMBIE.

The process has terminated but its parent
has not collected its exit status.

Parent:
  PID: 4182
  Name: server
  State: Sleeping

Recommendation:
Investigate parent PID 4182.
```

---

## 🌳 Process trees

Display the process hierarchy:

```bash
./deadproc tree <PID>
```

Example:

```text
systemd (PID 1) [Sleeping]
├── NetworkManager (PID 723) [Sleeping]
├── sshd (PID 1042) [Sleeping]
│   └── bash (PID 2104) [Sleeping]
│       └── deadproc (PID 4217) [Running]
└── my-service (PID 1302) [Sleeping]
    ├── worker (PID 1303) [Sleeping]
    └── worker (PID 1304) [Zombie] ⚠ ZOMBIE
```

The tree is reconstructed using:

```text
PID
PPID
```

---

## 👨‍👩‍👦 Parent chain

Trace a process back through its parents:

```bash
./deadproc parents <PID>
```

Example:

```text
worker (PID 1304) [Zombie]
    →
my-service (PID 1302) [Sleeping]
    →
systemd (PID 1) [Sleeping]
```

This is particularly useful when investigating zombie processes.

---

## 📊 Process statistics

Get a system-wide process summary:

```bash
./deadproc stats
```

Example:

```text
Process Statistics
════════════════════════════════════

Total processes:          142
Running:                  3
Sleeping:                 132
Uninterruptible sleep:    2
Stopped:                  1
Zombie:                   4
Idle:                     0
Unknown:                  0
```

---

## 📈 Live zombie monitoring

Start the zombie monitor:

```bash
./deadproc watch
```

deadproc continuously scans `/proc` and reports newly detected and resolved zombies.

Example:

```text
DEADPROC — Live Zombie Monitor
════════════════════════════════════

Monitoring /proc... Press Ctrl+C to stop.

[NEW ZOMBIE] PID=5124 Name=worker PPID=5090
[RESOLVED] Zombie 5124 is no longer present.
```

Press:

```text
Ctrl+C
```

to stop monitoring.

---

## 🖥️ Process monitor

Run the live process monitor:

```bash
./deadproc monitor
```

The monitor tracks CPU usage and memory usage and displays the most CPU-intensive processes.

Example:

```text
DEADPROC MONITOR
════════════════════════════════════════════════════════════

PID     NAME                  CPU       MEM         STATE
────────────────────────────────────────────────────────────
4217    compiler              86.4      41232K      Running
2104    chrome                21.7      845120K     Sleeping
1302    server                 8.2      92344K      Sleeping
1       systemd                0.0       18240K     Sleeping

Zombies: 0    Refresh: 1s    Ctrl+C: exit
```

---

## 📂 File descriptor inspection

Inspect the open file descriptors of a process:

```bash
./deadproc fds <PID>
```

Example:

```text
Open File Descriptors
════════════════════════════════════

0 → /dev/pts/2
1 → /dev/pts/2
2 → /dev/pts/2
3 → /home/user/file.txt
4 → socket:[123456]

Total: 5
```

This information comes from:

```text
/proc/<PID>/fd/
```

---

## 🌎 Environment inspection

Display a process's environment:

```bash
./deadproc env <PID>
```

This reads:

```text
/proc/<PID>/environ
```

Access may be restricted depending on the process and system permissions.

---

## 💻 Command line

Display the command line used to start a process:

```bash
./deadproc cmdline <PID>
```

This reads:

```text
/proc/<PID>/cmdline
```

---

## 🕒 Zombie history

deadproc maintains a persistent record of zombie processes.

View it using:

```bash
./deadproc history
```

Example:

```text
Zombie History
════════════════════════════════════════════════════════════

PID 4217  worker  parent=4182  first_seen=...  status=resolved
PID 5124  worker  parent=5090  first_seen=...  status=active

History file: ~/.cache/deadproc/zombies.log
```

---

# Architecture

The project is divided into several components:

```text
deadproc/
│
├── include/
│   ├── Process.hpp
│   ├── ProcessStats.hpp
│   ├── ProcessTree.hpp
│   ├── ProcessScanner.hpp
│   ├── ProcessMonitor.hpp
│   ├── ZombieHistory.hpp
│   └── Formatter.hpp
│
├── src/
│   ├── Process.cpp
│   ├── ProcessScanner.cpp
│   ├── ProcessMonitor.cpp
│   ├── ZombieHistory.cpp
│   ├── Formatter.cpp
│   └── main.cpp
│
├── Makefile
├── README.md
└── .gitignore
```

### `Process`

Responsible for loading information about an individual process.

It reads:

```text
/proc/<PID>/status
/proc/<PID>/stat
/proc/<PID>/io
/proc/<PID>/cmdline
/proc/<PID>/fd/
/proc/<PID>/environ
```

### `ProcessScanner`

Responsible for discovering processes by scanning:

```text
/proc
```

It also provides:

* zombie detection
* process statistics
* parent-chain construction
* process-tree construction

### `ProcessMonitor`

Responsible for calculating process CPU usage from process CPU ticks and system uptime.

### `ZombieHistory`

Tracks zombies across monitoring sessions and stores their history under:

```text
~/.cache/deadproc/
```

### `Formatter`

Responsible for converting process information into human-readable terminal output.

---

# Linux `/proc`

One of the main reasons for building this project was to understand how Linux exposes process information.

A simplified view:

```text
                Linux Kernel
                     │
                     ▼
                  /proc
                     │
       ┌─────────────┼─────────────┐
       ▼             ▼             ▼
   /proc/1       /proc/4217    /proc/5124
       │             │             │
       ▼             ▼             ▼
   process        process       process
   metadata       metadata      metadata
```

Each process normally gets a directory:

```text
/proc/<PID>/
```

For example:

```text
/proc/4217/status
/proc/4217/stat
/proc/4217/cmdline
/proc/4217/fd/
```

deadproc reads these interfaces directly.

---

# Building

## Requirements

* Linux
* GCC/G++
* C++20
* GNU Make

Check your compiler:

```bash
g++ --version
```

Check Make:

```bash
make --version
```

---

## Compile

Clone the repository and enter the project:

```bash
git clone <your-repository-url>
cd deadproc
```

Build:

```bash
make
```

This produces:

```text
./deadproc
```

---

# Usage

### Scan for zombies

```bash
./deadproc
```

### Inspect a process

```bash
./deadproc inspect <PID>
```

### Show process tree

```bash
./deadproc tree <PID>
```

### Show parent chain

```bash
./deadproc parents <PID>
```

### Diagnose process

```bash
./deadproc diagnose <PID>
```

### Inspect file descriptors

```bash
./deadproc fds <PID>
```

### Inspect environment

```bash
./deadproc env <PID>
```

### Show command line

```bash
./deadproc cmdline <PID>
```

### Process statistics

```bash
./deadproc stats
```

### Live zombie monitoring

```bash
./deadproc watch
```

### Live CPU/memory monitoring

```bash
./deadproc monitor
```

### Zombie history

```bash
./deadproc history
```

---

# 🧪 Testing Zombie Detection

You can intentionally create a zombie process for testing.

### Terminal 1

Start deadproc:

```bash
./deadproc watch
```

### Terminal 2

Run:

```bash
python3 -c 'import os,time; pid=os.fork(); os._exit(0) if pid==0 else (print(f"Zombie PID: {pid}", flush=True), time.sleep(30))'
```

The child exits immediately while the parent stays alive without calling `wait()`.

deadproc should report:

```text
[NEW ZOMBIE] PID=XXXXX Name=python3 PPID=XXXXX
```

You can then inspect it:

```bash
./deadproc inspect <PID>
```

or:

```bash
./deadproc diagnose <PID>
```

---

# ⚠️ Permissions

Some `/proc` information may not be accessible for every process.

For example:

```text
/proc/<PID>/environ
/proc/<PID>/fd/
```

may be restricted depending on:

* process ownership
* Linux security settings
* mount options
* ptrace restrictions

deadproc therefore treats inaccessible `/proc` entries as normal runtime conditions rather than assuming every process is readable.

---

# What I Learned

Building deadproc has been an exercise in understanding Linux from below the usual abstraction layer.

Some of the concepts explored:

* Linux process lifecycle
* PID and PPID
* Zombie processes
* `fork()`
* `exit()`
* `wait()` / `waitpid()`
* Process states
* `/proc`
* CPU accounting
* Process memory
* File descriptors
* Process environments
* Context switches
* Process hierarchies
* Userspace system monitoring

The interesting part is that a lot of what tools such as `ps` and `top` display ultimately comes from kernel-exposed interfaces such as `/proc`.

---

# Roadmap

deadproc is still under development.

Planned improvements:

* [ ] Better CPU percentage calculation and semantics
* [ ] CPU usage thresholds
* [ ] Memory usage thresholds
* [ ] Configurable alerts
* [ ] Process filtering
* [ ] Sort by CPU
* [ ] Sort by memory
* [ ] Filter by process state
* [ ] Filter by user
* [ ] Filter by process name
* [ ] JSON output
* [ ] Better zombie identity tracking
* [ ] PID reuse handling
* [ ] Automated tests
* [ ] Dedicated zombie test generator
* [ ] Install target
* [ ] Linux man page
* [ ] More robust CLI argument parsing

---

# Why deadproc?

The project started from a simple question:

> **"What actually happens when a Linux process becomes a zombie?"**

Instead of just reading about it, I wanted to build something that could observe it.

That turned into a much larger exploration of Linux process management and the `/proc` filesystem.

---

# License

Add your preferred license here.

For example:

```text
MIT License
```

---

## Author

**Shashank Tiwari**

Built with C++20 and Linux.
