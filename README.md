# OS_Project

**🌡️ Multi-Threaded Sensor Monitoring System using Producer-Consumer Model**

**📋 Overview**

This project implements a multi-threaded **Sensor Monitoring System** in C, demonstrating key **Operating Systems concepts**, especially the **Producer-Consumer Problem** with bounded buffer using **POSIX threads**, **semaphores**, and **mutexes**.

**✅ Core Concepts Demonstrated**

- Producer-Consumer Problem
- Bounded Buffer Synchronization
- POSIX Threading (pthread)
- Mutex Locks for Shared Resources
- Semaphore Signaling for Buffer Management
- Real-time Data Aggregation and Logging

**🧠 System Description**

This project simulates **three sensor types**:

- Temperature
- Humidity
- Motion

Each sensor is implemented as a **producer thread** that continuously generates and submits sensor readings into a **bounded buffer**.

A separate **consumer thread** (aggregator) reads from the buffer, computes averages every 10 readings per sensor type, and logs the output in a CSV file and to the console.

**🔧 Features**

**✅ Sensor Operations**

- Simulated sensor data generation
- Circular bounded buffer (FIFO queue)
- Live console display of incoming sensor data (colored output)
- Periodic average calculation and CSV logging

**✅ Synchronization**

- **Semaphores** used for buffer slot control (empty, full)
- **Mutexes** for protecting:
  - Shared buffer
  - Console output
  - File writes

**✅ Threading**

- 3 Producer threads (Temperature, Humidity, Motion)
- 1 Consumer thread (Aggregator)
- Each thread runs in an infinite loop

**🧵 Thread & Resource Control**

- Each thread is pthread\_created
- Buffer accesses are protected using pthread\_mutex\_lock
- File and console writes are protected to avoid race conditions
- sem\_wait and sem\_post handle buffer slot management

**📄 Output Format**

**CSV File: sensor_averages.csv**

Timestamp,SensorType,AverageValue,RecordCount

"2025-05-08 12:34:00","Temperature",27.53,10

...

**Console Output:**

[2025-05-08 12:34:01] Temperature: 25.30

[2025-05-08 12:34:02] Humidity: 62.00

...

[2025-05-08 12:34:10] AVERAGE Temperature: 27.53 (10 samples)

**📁 File Structure**

├── sensor\_system.c     # Main source file with threading and logic

├── sensor\_averages.csv # Output log file for sensor averages

**🔗 OS Concepts Used**

|**Concept**|**Implementation Example**|
| :- | :- |
|Producer-Consumer|Producer (sensor) and consumer (aggregator) threads|
|Bounded Buffer|Circular queue of size 10|
|Mutex|Lock access to shared resources|
|Semaphores|Control availability of buffer slots|
|File I/O|CSV writing with fopen / fprintf|
|Console Sync|ANSI color output with mutex|

**🚀 Build & Run Instructions**

\# Compile

gcc -o sensor\_system sensor\_system.c -lpthread

\# Run

./sensor\_system

**👨‍💻 Authors**

✍️ Developed by : Rao Ghulam

`                     `Sheikh Naveed

`                     `Syed Wirad Uddin

**📌 Notes**

- The program runs indefinitely for live simulation.
- Can be extended to include more sensors or visualization.
- Useful for learning synchronization and real-time data handling in OS.
