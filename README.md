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
│
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

## ⚙️ How to Compile

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

##  How Simulation Works

### 1. Initialization
- Teams and match parameters are set
- Threads are created for different components

### 2. Ball Outcome Generation
Each ball uses a probability model:

```c
r = rand() % 100;
```

Mapped outcomes:
- Runs (0–6)
- Wickets
- Extras

---

### 3. Scheduling
- Events are scheduled using custom logic
- Simulates OS scheduling (e.g., FCFS / Round Robin)

---

### 4. Synchronization
- Shared resources (scoreboard, stats) are protected using:
  - Mutex locks  
  - Semaphores  

---

### 5. Deadlock Handling
- Detects circular wait conditions
- Applies resolution strategy to avoid system halt

---

### 6. Logging & Output
- Ball-by-ball commentary  
- Final scorecard  
- Execution timeline (Gantt-style output)  

---

## 📊 Sample Output

- Over-wise commentary  
- Total runs, wickets  
- Match summary  
- Execution order visualization  

---

##  Assumptions

- Probability-based outcomes (pseudo-random)  
- Simplified T20 rules  
- Fixed 20 overs  
- No real-time delay (logical simulation only)  

---

##  Possible Improvements

- GUI or web-based visualization  
- Tournament simulation (multiple matches)  
- AI-based probability model  
- Real player statistics integration  
- Comparison of multiple scheduling algorithms  

---

##  Learning Outcomes

- Practical understanding of multithreading  
- Synchronization and race condition handling  
- Deadlock detection and resolution  
- OS scheduling in a real-world scenario  

---

##  License

This project is intended for academic and educational purposes.

---

##  Author

Developed as part of an Operating Systems course project.
