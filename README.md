# GRS_PA02
# MT25090 Assignment2

##  Overview

This assignment evaluates the performance impact of different data copy mechanisms in socket-based communication:

- **A1 – Two-Copy (Baseline)**
- **A2 – One-Copy**
- **A3 – Zero-Copy**

The goal is to analyze throughput, latency, CPU cycles, and cache behavior using Linux `perf` hardware performance counters.

---

## System Configuration

- OS: Ubuntu 22.04 LTS
- Compiler: GCC (-O2 -pthread)
- Measurement Tool: `perf stat`
- Run Duration: 5 seconds per experiment
- Threads Tested: 1, 2, 4, 8
- Message Sizes: 1KB, 4KB, 16KB, 64KB

---
## Compilation
- make

- This compiles:

MT25090_Part_A1_Server

MT25090_Part_A2_Server

MT25090_Part_A3_Server

MT25090_Part_A_Client

## Running Experiments

To automatically run all experiments:

chmod +x MT25090_Part_C_Bash.sh
./MT25090_Part_C_Bash.sh

This will:

- Run all combinations of message sizes and thread counts

- Collect perf statistics

- Generate MT25090_Part_C_Expresults.csv

## Generating plots
- Activate virtual environment (if used):

source venv/bin/activate

Run:

python3 MT25090_Part_D_plots.py


Generated plots:

- throughput_vs_message_size.png

- latency_vs_threads.png

- cache_misses_vs_message_size.png

- cycles_per_byte.png

## Metrics Collected
perf stat -e cycles,L1-dcache-load-misses,LLC-load-misses,context-switches

Measured metrics:

Throughput (Gbps)

Latency (µs)

CPU Cycles

L1 Cache Misses

LLC Cache Misses

Context Switches
