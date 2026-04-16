# Cache Controller FSM Simulator

## Description
This project simulates a finite state machine based simple cache controller in C++.

## Cache Characteristics
- Direct-mapped cache
- Write-back
- Write-allocate
- Block size: 4 words (128 bits)
- 1024 cache lines
- 32-bit addresses

## FSM States
- IDLE
- COMPARE_TAG
- WRITE_BACK
- ALLOCATE

## Build
```bash
g++ -std=c++17 simulator.cpp -o simulator
