#  T20 Cricket Simulator (OS-Based Project)

A multi-threaded T20 cricket simulator built to demonstrate core **Operating System concepts** such as scheduling, synchronization, concurrency, and deadlock handling through a realistic cricket match environment.

---

##  Overview

This project models a T20 cricket match where different components (players, match engine, logging system) execute concurrently using threads. It simulates ball-by-ball gameplay while ensuring safe access to shared resources.

---

##  Features

- Ball-by-ball T20 match simulation  
- Multi-threaded architecture  
- Custom scheduling mechanism  
- Synchronization using mutex/semaphores  
- Deadlock detection and resolution  
- Detailed logging system  
- Gantt chart visualization of execution  
- Match statistics and scorecard generation  

---

##  Project Structure

```bash
t20-os-cricket-simulator/
│
├── include/
│   └── simulator.h          # Header file (structures + declarations)
│── docs/
│   └── Cricket world cup simulator report.pdf
│   └── demo_video.mp4     
├── src/
│   ├── main.c               # Entry point of the program
│   ├── match_engine.c       # Core match simulation logic
│   ├── scheduler.c          # Scheduling algorithms
│   ├── threads.c            # Thread creation & management
│   ├── sync.c               # Synchronization (mutex/semaphores)
│   ├── stats.c              # Match statistics tracking
│   ├── logger.c             # Event logging system
│   ├── gantt.c              # Gantt chart generation
│
├── simulator                # Compiled executable (after build)
├── README.md
```

---

##  Clone the repository

```bash
git clone https://github.com/Perpetu4l/t20-os-cricket-simulator.git
cd t20-os-cricket-simulator
```

---

##  How to Compile

Make sure you have **GCC** and **pthread library** installed.

```bash
gcc src/*.c -Iinclude -o simulator -lpthread
```

---

##  How to Run

```bash
./simulator
```

---
## Demo Video 
- [View Demo Video](https://drive.google.com/file/d/1_AYMzddsqNbVvnkcUkUmzYepnugNuu54/view?usp=sharing)
- Also present in `./docs`
---
## Report
- [View Report](https://drive.google.com/file/d/1MZpy0QVuU0buePMO4rKyeXKWB_Nv4nol/view?usp=drive_link)
- Also present in `./docs`
---
---
## Note
This project was developed collaboratively by all group members. Since the implementation and testing were carried out on a single Linux laptop, the Git commit history primarily reflects contributions from that machine and may not accurately represent the individual coding contributions of each team member.
