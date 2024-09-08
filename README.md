# Cache Simulator Project

## Team
- **Chengcheng Zhang**
- **Stuart Boynton**

## Project Overview
This project, implemented in **C++**, is a part of our **Computer Architecture** course. It simulates a **Cache Simulator** that models the behavior of a processor cache, including Instruction cache (I-Cache), Data cache (D-Cache), and Level 2 cache (L2-Cache). The simulator supports **inclusive** and **non-inclusive** L2 cache configurations and two prefetching methods: **Next-line Prefetching** and a custom **Stride Prefetching** algorithm. Our code follows the Least Recently Used (LRU) replacement policy.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
   - LRU Algorithm
   - Prefetch Algorithms
3. [Simulation Results](#simulation-results)
   - No Prefetching
   - With Prefetching
   - Effect of Set Associativity
4. [Running the Code](#running-the-code)
5. [Conclusion](#conclusion)

---

## Introduction

The goal of this project, part of our **Computer Architecture** coursework, is to simulate the performance of a cache hierarchy using given traces of real programs. The simulator models the cache's behavior, measures total access time, and evaluates the impact of various configurations and optimization techniques (e.g., prefetching). Understanding these behaviors is crucial for designing efficient and high-performing computer architectures.

---

## Features

### 1. LRU Algorithm
We implemented a simple **Least Recently Used (LRU)** algorithm for cache block replacement. Each block at a given index in the cache is assigned a rank based on how recently it was accessed. 

- The block accessed most recently is assigned the highest rank.
- When a block is accessed, its rank is updated, and the ranks of other blocks are decremented accordingly.

This ensures that the least recently used block is evicted first when the cache is full.

### 2. Prefetch Algorithms
- **Next-Line Prefetching:** This is a simple technique where the cache prefetches the next block after the current one, assuming sequential memory access.
  
- **Stride Prefetching:** Our custom algorithm detects a pattern in memory access by calculating the stride (difference in address) between consecutive accesses. If a consistent stride is detected, the next block is prefetched based on this stride.

---

## Simulation Results

### 1. Total Access Time (Without Prefetching)

| Trace  | MIPS (Non-Inclusive) | MIPS (Inclusive) | Alpha (Non-Inclusive) | Alpha (Inclusive) |
|--------|----------------------|------------------|-----------------------|-------------------|
| bzip2  | 222,402,900           | 222,863,250      | 362,465,750           | 362,515,350       |
| gcc    | 48,918,100            | 49,778,800       | 47,036,100            | 47,036,100        |
| h264   | 45,119,150            | 45,332,300       | 45,986,050            | 45,986,050        |
| namd   | 44,938,800            | 45,050,000       | 42,452,350            | 42,452,350        |

### 2. Total Access Time (With Prefetching)

| Trace  | MIPS (Next-Line) | MIPS (Stride) | Alpha (Next-Line) | Alpha (Stride) |
|--------|------------------|---------------|-------------------|----------------|
| bzip2  | 73,326,550        | 73,325,700    | 73,097,150        | 73,096,000     |
| gcc    | 45,678,650        | 45,697,750    | 41,838,250        | 41,897,800     |
| h264   | 41,996,800        | 41,995,050    | 41,084,750        | 41,081,250     |
| namd   | 44,164,200        | 44,163,550    | 40,989,500        | 40,987,950     |

### 3. Increasing Set Associativity (Alpha A21264)

**Simulation results with increasing set associativity (on `bzip2` trace):**

| Set Associativity | Total Access Time (D-Cache) |
|-------------------|----------------------------|
| 1                 | 363,568,900                |
| 2                 | 362,467,950                |
| 4                 | 362,465,750                |
| 8                 | 362,465,450                |
| 16                | 362,465,400                |

- **Observation:** As the set associativity increases, the total access time decreases. However, higher set associativity comes with increased hardware complexity and slower hit times, as multiple tags must be compared to locate the requested block.


---

## Running the Code

To compile and run the simulator, follow these steps:

1. Clone the repository:
   ```
   git clone https://github.com/MerlinZCC/Cache-Simulator.git
   ```
2. Navigate to the `src` directory and compile the code:
   ```
   cd src
   make all
   ```
3. Run the simulator with appropriate options:
   ```
   bunzip2 -kc trace.bz2 | ./cache --icache=<sets:assoc:blocksize:hit> --dcache=<sets:assoc:blocksize:hit> --l2cache=<sets:assoc:blocksize:hit> --memspeed=<latency>
   ```

### Example:
To run the simulator with **MIPS R10K** configuration:
```
./cache --icache=128:2:128:2 --dcache=64:4:128:2 --l2cache=128:8:128:50 --memspeed=100
```

## Conclusion

This project has deepened our understanding of cache mechanisms and their impact on processor performance, a critical aspect of **Computer Architecture**. By simulating various configurations and prefetching techniques, we gained valuable insights into how different design choices affect cache efficiency. This knowledge is essential for optimizing performance in real-world applications.
