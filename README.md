# Cache Controller FSM Simulator (C++)

## Description

This project implements a finite state machine (FSM)-based cache controller based on Patterson & Hennessy (Computer Organization and Design).
The simulator models a direct-mapped cache with write-back and write-allocate policies and shows cycle-by-cycle state transitions.

---

## Cache Characteristics

* Direct-mapped cache
* 1024 cache lines
* Block size: 4 words (128 bits)
* 32-bit address
* Write-back policy
* Write-allocate policy

---

## FSM States

* IDLE
* COMPARE_TAG
* WRITE_BACK
* ALLOCATE

---

## Requirements

* C++ compiler (g++ recommended)

---

## Build

```bash
g++ -std=c++17 simulator.cpp -o simulator
```

---

## Run

```bash
./simulator
```

---

## Test

The simulator uses a predefined sequence of requests inside the code.

Test cases:

* TC1: Read miss (cold start)
* TC2: Read hit
* TC3: Write hit
* TC4: Dirty miss (write-back required)

You can verify correctness by observing:

* FSM state transitions
* Hit/miss behavior
* Memory delay during WRITE_BACK and ALLOCATE

---

## Example Output

Generate output file:

```bash
./simulator > example_output.txt
```

---

## Report

See `REPORT.pdf` for the full explanation.

---

## Author / Group

* AKM Azimul Ashique Khan(ID:230041153)
* Mahir Labib(ID:230041139)
* Abrar Faiyaz(ID:230041139)

