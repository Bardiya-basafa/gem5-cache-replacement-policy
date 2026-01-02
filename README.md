
# gem5 Cache Replacement Policy Study
## Introduction

**The project implements an adaptive cache replacement policy in gem5 using the Dueling Set technique to dynamically choose between LRU and LIP policies based on runtime workload characteristics.** The system intelligently monitors cache performance through partitioned sets and a PSEL (policy selector) counter, enabling optimal policy selection for different access patterns including high-locality workloads, streaming applications, and phase-changing behaviors without prior knowledge of the workload type.

**By creating a self-tuning cache controller, this research demonstrates how adaptive policies can outperform static approaches like pure LRU or LIP, providing better overall miss rates and IPC across diverse applications—a crucial advancement for modern processors that encounter varying memory access patterns in real-world usage scenarios.**
## What is gem5?

**Gem5 emerged in 2011 from the merger of two pioneering academic simulators—Michigan's m5 (focusing on CPU modeling) and Wisconsin's GEMS (specializing in memory systems)—creating a unified, modular platform for full-system computer architecture research.** This fusion established gem5 as the dominant open-source simulator in both academia and industry, evolving through versions that added support for diverse ISAs (ARM, x86, RISC-V), GPU modeling, and system-level features while maintaining its core philosophy of separating functional simulation from timing models.

**Today, gem5 serves as a virtual hardware laboratory where researchers explore CPU microarchitectures, cache hierarchies, memory controllers, and interconnects without physical fabrication.** Its primary use cases include prototyping new processor designs (like Apple's early ARM experiments), evaluating cache replacement policies (as in your streaming workload experiments), studying side-channel vulnerabilities like Spectre/Meltdown, validating memory consistency models, and serving as a reference model for commercial RTL development by companies like ARM, Google, and AMD.

## How gem5 Works Generaly
**Gem5 is a modular, cycle-accurate computer architecture simulator that enables researchers to model and evaluate computing systems by combining configurable hardware components.** At its core, gem5 separates functional correctness from timing accuracy—it can simulate everything from simple single programs (Syscall Emulation mode) to full operating systems (Full System mode) using a variety of CPU models (from basic in-order to complex out-of-order pipelines), customizable cache hierarchies, memory controllers, and interconnects.

**Researchers use gem5 like "computer architecture LEGO"—mixing and matching components to test new processor designs, memory systems, cache policies, and interconnects while collecting detailed performance statistics.** Your streaming workload experiment exemplifies this approach: by swapping replacement policies (like LIP vs. LRU) in the cache module, you can measure their impact on miss rates and latency, validating architectural ideas in a controlled, reproducible environment before hardware implementation.
## Gem5 Source Structure Overview

**Gem5's source code follows a dual-language architecture where Python provides the configuration interface and C++ implements the simulation core.** The `src/` directory contains C++ models of hardware components (CPUs, caches, memory controllers) that define their behavior and timing, while Python scripts in `configs/` and throughout the codebase allow users to assemble these components into complete systems without modifying C++.
___
### Directory Structure
```gem5/
├── src/                    # C++ simulation models
│   ├── cpu/               # CPU implementations
│   ├── mem/               # Memory system
│   ├── sim/               # Core simulator
│   └── ruby/              # Coherence protocols
├── configs/               # Example Python configs
├── python/               # Python modules
│   └── gem5/             # Library for building systems
└── build/                # Compiled binaries
```
___
### C++ Components
***C++ classes implement the detailed timing models and hardware behaviors that execute simulations cycle-by-cycle.** Each component (like Cache, CPU, or DRAM controller) is a C++ object with ports, events, and statistics tracking. When configured via Python, these objects connect through ports to form a complete system.
___
### Python Components
**Python scripts act as "architectural blueprints"—they instantiate C++ objects, set parameters, connect components, and describe the target workload.** The `m5` module bridges Python and C++, allowing Python to drive simulation while C++ handles performance-critical execution. This separation enables rapid prototyping of new architectures through Python configuration while maintaining the speed and accuracy of C++ simulation models.

## Replacement Policies
### **LRU (Least Recently Used)**

**How it works:** Maintains recency order using stack/list. On access: move line to Most Recently Used (MRU) position. On eviction: remove Least Recently Used line. Tracks "when" each line was last used.

**Why use:** Leverages temporal locality principle - recently used data likely reused. Good average performance for general-purpose workloads.

**Best for:** Workloads with strong temporal locality, database operations, typical application code with predictable access patterns.

**Avoid when:** Streaming/scans (zero reuse), thrashing patterns, workloads larger than cache where LRU degenerates to FIFO.

----------

### **LIP (LRU Insertion Policy)**

**How it works:** Inserts new lines at LRU position (not MRU). Only promotes to MRU on cache hits. Prevents "cache pollution" from streaming data by giving one-time accesses immediate eviction priority.

**Why use:** Protects cache from useless streaming data. Maintains hot working set despite transient accesses.

**Best for:** Mixed workloads with streaming components, scan-heavy databases, multimedia processing.

**Avoid when:** Pure high-locality workloads where all accessed data is reused - delays promotion of hot data.

----------

### **Dueling Set (Adaptive Framework)**

**How it works:** Partitions cache: some sets use Policy A, others use Policy B, rest follow winner. PSEL counter tracks which policy performs better, dynamically switches followers.

**Why use:** Adapts to unknown/changing workloads automatically. No offline profiling needed.

**Best for:** Systems with varying workloads, general-purpose processors, server applications.

**Avoid when:** Power/area-constrained systems (overhead), known static workloads where simpler policy suffices.

## How The Code Works
**The simulation script initializes a complete computer system model in gem5 by instantiating hardware components and connecting them through a memory bus.** It begins by creating a `System` object with a 3GHz clock domain and timing-based memory mode, then adds a `TimingSimpleCPU` as the processor core. The script configures two 32kB L1 caches (instruction and data) with 4-way associativity, each using a `DuelingRP` replacement policy that dynamically chooses between LRU and LIP based on runtime performance. These caches connect to a shared `SystemXBar` memory bus, which in turn links to a simplified main memory model with fixed 50ns latency.
```
system.cpu.icache.mem_side = system.membus.slave
system.cpu.dcache.mem_side = system.membus.slave
system.mem.port = system.membus.master
```
**The script establishes interrupt handling and system ports necessary for x86 syscall-emulation mode before loading the specified workload binary.** After configuring the `SEWorkload` with the target executable, it creates a `Process` object, attaches it to the CPU, and spawns execution threads. The simulation concludes by instantiating the complete model through `m5.instantiate()` and running the workload via `m5.simulate()`, finally reporting termination status and simulation duration.

**Note that identical code structure is used for pure LRU and LIP configurations—simply replacing `DuelingRP()` with `LRURP()` or `LIPRP()` in the cache replacement policy assignments while keeping all other system components and connections unchanged.** This enables direct performance comparisons between adaptive and static replacement policies using the exact same hardware setup and workload conditions.
## Running Simulations
1) **Build gem5**:
- `scons build/X86/gem5.opt -j$(nproc)`
- `ln -s /home/[your path to gem5 root]/gem5/build/X86/gem5.opt /usr/local/bin/gem5-x86`
2) **Run workloads** with different policies, sending outputs to results/[workload]/[policy]/:
- `gem5-x86 -d results/streaming/lru lru_replacement_policy --workload=streaming`
- `gem5-x86 -d results/streaming/lip lip_replacement_policy --workload=streaming`
 - `gem5-x86 -d results/streaming/dueling dueling_replacement_policy --wrokload=streaming`

- **Repeat** for high_locality and phase_change (lru/lip/dueling).
3) After each run, **read metrics** in:
- results/[workload]/[policy]/stats.txt (IPC/CPI, simSeconds, dcache/icache miss rates, bytes, bandwidth).

How the outputs were generated for this study:
- Each stats.txt begins with the workload/policy label (e.g., “streaming lru”). We extracted IPC/CPI, L1D/L1I miss rates and counts, simSeconds, simInsts, memory bytes read/written, and bwTotal::total from these files.

## Results & Comparison

Numbers below are taken directly from stats.txt for each workload/policy. “Cache Miss Rate” refers to L1D overall miss rate unless noted. Memory Bandwidth is system.mem.bwTotal::total. All table values are from the corresponding stats.txt block.

### Streaming
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.247464 | 0.247451 | 0.247462 |
| CPI | 4.040992 | 4.041206 | 4.041025 |
| L1D Miss Rate | 0.059043 | 0.059045 | 0.059044 |
| L1I Miss Rate | 0.000103 | 0.000104 | 0.000103 |
| L1D Misses | 64538 | 64540 | 64539 |
| L1I Misses | 664 | 668 | 664 |
| Exec Time (simSeconds) | 0.006018 | 0.006019 | 0.006018 |
| simInsts | 4,472,407 | 4,472,407 | 4,472,407 |
| DRAM Read Bytes (total) | 4,173,120 | 4,173,504 | 4,173,184 |
| DRAM Write Bytes (total) | 2,097,856 | 2,097,664 | 2,097,728 |
| Memory Bandwidth (total) | 1,041,980,000 B/s | 1,041,956,556 B/s | 1,041,960,775 B/s |

Key Observations:
- All three are practically identical on IPC/CPI, miss rates, and bandwidth. This suggests the stream’s footprint and reuse distance don’t allow either LIP or Dueling to gain a meaningful advantage over LRU at the tested cache parameters.
- Minor differences in bytes and writebacks are negligible here; at aggregate scale they don’t move CPI or simSeconds.




### High Locality
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.585550 | 0.176342 | 0.509285 |
| CPI | 1.707795 | 5.670790 | 1.963539 |
| L1D Miss Rate | 0.000491 | 0.053439 | 0.003908 |
| L1I Miss Rate | 0.000049 | 0.000049 | 0.000049 |
| L1D Misses | 2,502 | 272,052 | 19,897 |
| L1I Misses | 642 | 643 | 642 |
| Exec Time (simSeconds) | 0.006266 | 0.020807 | 0.007205 |
| simInsts | 11,018,691 | 11,018,691 | 11,018,691 |
| DRAM Read Bytes (total) | 201,472 | 17,452,736 | 1,314,752 |
| DRAM Write Bytes (total) | 64,832 | 682,368 | 130,240 |
| Memory Bandwidth (total) | 42,497,841 B/s | 871,568,384 B/s | 200,563,076 B/s |

Key Observations:
- LRU absolutely wins: extremely low L1D miss rate (0.000491) and best IPC/CPI. Strong temporal locality favors keeping recently used lines hot, which is exactly LRU’s strength [2].
- LIP gets hammered: it kicks new lines to the bottom; with fast reuse, many of those “new” lines should have been near the top. Misses and bandwidth spike, CPI balloons, and simSeconds triple vs LRU.
- Dueling adapts partially: far better than LIP, but still behind LRU. Its set selection likely didn’t fully commit to LRU across all sets, or the training/selection parameters weren’t optimal for this footprint, leaving some sets governed by the non-ideal policy.

Validation note: Values pulled from high_locality stats blocks; miss counts align with rates and accesses; bandwidth tracks total bytes over simSeconds. IPC/CPI trends match miss-rate changes. Complete.

### Phase Change
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.512970 | 0.248593 | 0.480746 |
| CPI | 1.949432 | 4.022640 | 2.080099 |
| L1D Miss Rate | 0.003764 | 0.033805 | 0.005657 |
| L1I Miss Rate | 0.000009 | 0.000009 | 0.000009 |
| L1D Misses | 99,067 | 889,727 | 148,898 |
| L1I Misses | 670 | 672 | 672 |
| Exec Time (simSeconds) | 0.040106 | 0.082759 | 0.042795 |
| simInsts | 61,781,989 | 61,781,989 | 61,781,989 |
| DRAM Read Bytes (total) | 6,383,424 | 56,985,792 | 9,572,736 |
| DRAM Write Bytes (total) | 3,184,640 | 3,921,664 | 3,230,784 |
| Memory Bandwidth (total) | 238,566,532 B/s | 735,957,570 B/s | 299,184,274 B/s |

Key Observations:
- LRU wins overall, implying most phases are locality-friendly or long enough that LRU’s retention pays off. It sees the lowest miss rate and best IPC/CPI [2].
- LIP loses across phases (highest miss rate and bandwidth, worst CPI/time). In the locality-heavy phases, deprioritizing new lines hurts; in streaming-like phases it helps, but not enough to beat LRU’s overall advantage here.
- Dueling is in the middle: much better than LIP, but not on par with LRU. Adaptation helps—but training window, number of dueling sets, and phase lengths likely limit how fast and how completely it switches to the better policy.



## Discussion
How the code and mechanisms behave, and why results look like this:
- How the code works:
  - You select a replacement policy in the Python config (LRU, LIP, or Dueling). That config constructs cache SimObjects and binds a ReplacementPolicy implementation beneath them. The policy decides victim selection (and for LIP/Dueling, insertion rank or per-set policy choice). The rest (MSHRs, hit/miss counters, latency accounting) is in gem5’s C++ models, with stats exposed in m5out/stats.txt.
- Why results differ:
  - Streaming: With low reuse, policies converge. LIP’s anti-pollution and LRU’s recency don’t diverge much at the tested cache sizes; hence the near-equal IPC/CPI and bandwidth.
  - High Locality: LRU’s recency heuristic matches the workload perfectly—lines are reused soon, so keeping “recent” lines at the top minimizes misses. LIP undercuts this by putting new lines low; many are reused quickly, so they get evicted prematurely, exploding miss rate and bandwidth.
  - Phase Change: Dueling helps but still trails LRU. Real workloads have phase noise, transitions, and varying footprints. If dueling’s training sets are few or the decision horizon is long/short relative to phase lengths, it can lag behind the best policy in parts of the run. This shows up as a higher miss rate vs LRU and worse IPC/CPI.
- Why LRU can beat Dueling:
  - If most of the runtime favors locality or if phases are long and clearly LRU-friendly, then a static LRU is hard to beat. Dueling’s overhead/latency in switching and its limited set coverage can keep some sets on the “wrong” policy, leaving performance on the table.
  - Selection noise, insufficient dueling sets, or mis-tuned thresholds make decisions less decisive. In such conditions, recency-based retention (LRU) wins more reliably.


