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
│   └── REPORT.pdf
│   └── demo_video.mp4
│   └── presentation.pptx       
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
- [View Report](https://drive.google.com/file/d/1Hr32ECVocaf2WK6lC0zC6ugZL30L7fHy/view?usp=sharing)
- Also present in `./docs`
---
## Presentation
- [View Presentation](https://docs.google.com/presentation/d/1cZFL2dq5qcgaY9d8uzKdMGl0z5SOwI38/edit?usp=sharing&ouid=107112480013412523230&rtpof=true&sd=true)
- Also present in `./docs`
---
