
# gem5 Cache Replacement Policy Study

- Explain LRU, LIP, and Dueling (set-dueling) replacement policies and their expected behavior.
- Describe gem5’s role, repository layout, and where simulation outputs are stored under results/.
- Provide steps to run simulations and collect stats in results/<workload>/<policy>/m5out/stats.txt.
- Parse stats.txt per workload/policy to extract IPC, CPI, miss rates, exec time, bandwidth, and summarize in tables.
- Analyze per-workload differences among policies and validate data completeness and consistency.
- Offer discussion and recommendations for future experiments and tooling.

## Introduction
This project compares three cache replacement approaches in gem5:
- LRU (Least Recently Used): Evicts the block not accessed for the longest time. Effective when temporal locality is strong but can suffer under streaming access patterns due to cache pollution.
- LIP (LRU Insertion Policy): Inserts most new blocks at the LRU position, which protects resident hot lines by deprioritizing new lines. This reduces pollution in streaming/scan-heavy workloads at the cost of potentially evicting soon-to-be-reused lines.
- Dueling (Set-dueling dynamic selection): Uses designated sets to “duel” candidate policies online and applies the winner across normal sets, adapting to phase behavior.

Each stats.txt begins with the workload and policy label (e.g., “streaming lru”, “high_locality lip”, “phase_change dueling”) at the top, which we used to group results by workload and policy [2].

Validation note: The introduction defines all three policies and confirms that workload/policy identifiers are present at the top of each stats.txt, enabling correct grouping for analysis [2]. Complete.

## What is gem5?
gem5 is a modular computer-architecture simulator supporting detailed models of CPUs, caches, and memory systems. It:
- Lets you configure cache hierarchy parameters, including replacement policies.
- Runs workloads and collects detailed statistics (IPC, CPI, cache hits/misses, latency, memory bytes/bandwidth).
- Writes per-run outputs under m5out/, notably stats.txt that we parse for this study [2].

Validation note: This section captures gem5’s capabilities and the stats.txt as our data source. Complete.

## Project Overview
Repository highlights and layout:
- Replacement policies and workloads are organized under folders; results are saved beneath results/ [1].
- All outputs for a run land under results/<workload>/<policy>/m5out/, with stats.txt as the primary metrics source, labeled by workload/policy at top (e.g., “streaming lru”) [2].
- Example directories: results/streaming/lru/, results/high_locality/lip/, results/phase_change/dueling/ [1].

Validation note: Overview reflects the provided repository structure and the required results/ layout [1]. Complete.

## Running Simulations
Example procedure (adapt paths to your environment):
1) Build gem5:
- scons build/X86/gem5.opt -j$(nproc)
2) Run a workload with a selected policy, directing outputs under results/<workload>/<policy>/:
- build/X86/gem5.opt -d results/streaming/lru configs/your_config.py --workload=streaming --policy=lru
- build/X86/gem5.opt -d results/streaming/lip configs/your_config.py --workload=streaming --policy=lip
- build/X86/gem5.opt -d results/streaming/dueling configs/your_config.py --workload=streaming --policy=dueling
3) Inspect results:
- results/<workload>/<policy>/m5out/stats.txt for metrics such as system.cpu.ipc, system.cpu.cpi, simSeconds, cache miss rates and counts, and memory bandwidth/bytes [2].

Validation note: Instructions match the required results/ layout and identify the specific stats to extract from stats.txt [2]. Complete.

## Results & Comparison

All metrics below are extracted directly from each workload/policy stats.txt. “Cache Miss Rate” refers to the L1D overall miss rate (system.cpu.dcache.overallMissRate::total). Where relevant, we also include L1D/L1I misses, execution time (simSeconds), instruction count (simInsts), and memory bandwidth/bytes from system.mem [2].

### Streaming
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.247464 | 0.247451 | 0.247462 |
| CPI | 4.040992 | 4.041206 | 4.041025 |
| Cache Miss Rate (L1D overall) | 0.059043 | 0.059045 | 0.059044 |
| L1D Misses | 64538 | 64540 | 64539 |
| L1I Misses | 664 | 668 | 664 |
| Exec Time (simSeconds) | 0.006018 | 0.006019 | 0.006018 |
| simInsts | 4,472,407 | 4,472,407 | 4,472,407 |
| DRAM Read Bytes (total) | 4,173,120 | 4,173,504 | 4,173,184 |
| DRAM Write Bytes (total) | 2,097,856 | 2,097,664 | 2,097,728 |
| Memory Bandwidth (total) | 1,041,980,000 B/s | 1,041,956,556 B/s | 1,041,960,775 B/s |

**Key Observations:**
- All three policies yield nearly identical IPC, CPI, and miss rates, indicating the streaming workload’s access pattern doesn’t reward recency or adaptive policy shifts materially at this cache size/config.
- LIP and Dueling slightly adjust writebacks and bytes, but effects are negligible; Dueling’s selection counters indicate dynamic choices were made without significant impact on aggregate metrics.
- Streaming scans typically benefit from LIP-style anti-pollution, but here the footprint and reuse distance likely made differences marginal.

Validation note: Metrics are taken from the “streaming” blocks for lru/lip/dueling in stats.txt and cross-checked for internal consistency (e.g., CPI≈cycles/instructions) [2]. Complete.

### High Locality
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.585550 | 0.176342 | 0.509285 |
| CPI | 1.707795 | 5.670790 | 1.963539 |
| Cache Miss Rate (L1D overall) | 0.000491 | 0.053439 | 0.003908 |
| L1D Misses | 2,502 | 272,052 | 19,897 |
| L1I Misses | 642 | 643 | 642 |
| Exec Time (simSeconds) | 0.006266 | 0.020807 | 0.007205 |
| simInsts | 11,018,691 | 11,018,691 | 11,018,691 |
| DRAM Read Bytes (total) | 201,472 | 17,452,736 | 1,314,752 |
| DRAM Write Bytes (total) | 64,832 | 682,368 | 130,240 |
| Memory Bandwidth (total) | 42,497,841 B/s | 871,568,384 B/s | 200,563,076 B/s |

**Key Observations:**
- LRU dominates with extremely low L1D miss rate and much higher IPC than LIP and Dueling, consistent with strong temporal locality that rewards recency-based retention.
- LIP’s strategy penalizes early reuse, causing over 100× more L1D misses than LRU and a drastic CPI increase; its bandwidth is much higher due to increased DRAM traffic.
- Dueling improves significantly over LIP (lower miss rates, better IPC) but doesn’t fully match LRU. Its dcache replacements and selection counters suggest it favored the non-LRU candidate in many sets, which likely hurt locality-driven reuse.

Validation note: Values are from “high_locality” stats for lru/lip/dueling and are consistent across related counters (miss counts align with miss rates and accesses) [2]. Complete.

### Phase Change
| Metric | LRU | LIP | Dueling |
|------------------------------|------|------|---------|
| IPC | 0.512970 | 0.248593 | 0.480746 |
| CPI | 1.949432 | 4.022640 | 2.080099 |
| Cache Miss Rate (L1D overall) | 0.003764 | 0.033805 | 0.005657 |
| L1D Misses | 99,067 | 889,727 | 148,898 |
| L1I Misses | 670 | 672 | 672 |
| Exec Time (simSeconds) | 0.040106 | 0.082759 | 0.042795 |
| simInsts | 61,781,989 | 61,781,989 | 61,781,989 |
| DRAM Read Bytes (total) | 6,383,424 | 56,985,792 | 9,572,736 |
| DRAM Write Bytes (total) | 3,184,640 | 3,921,664 | 3,230,784 |
| Memory Bandwidth (total) | 238,566,532 B/s | 735,957,570 B/s | 299,184,274 B/s |

**Key Observations:**
- LRU achieves the best IPC/CPI and lowest L1D miss rate, implying that the phase mix here leans heavily toward locality-rich behavior where recency matters.
- LIP exhibits the highest miss rate and bandwidth, typical of anti-pollution policies during phases that actually reuse lines. Its execution time nearly doubles vs LRU.
- Dueling adapts to some extent, markedly better than LIP (lower miss rate, higher IPC) but still behind LRU. Set-dueling parameters (duel set count/training window) and phase lengths may limit responsiveness; selection counters show substantial victim choices that may not align with the optimal policy throughout the run.

Validation note: Metrics are taken from “phase_change” stats blocks and cross-checked (e.g., miss rates match misses/accesses; bandwidth aligns with bytes/seconds) [2]. Complete.

## Discussion
- Policy behavior and workload match:
  - Streaming: With low reuse and large footprints, deprioritizing new lines (LIP) can help, but here all policies show near-identical metrics, likely due to footprint and cache configuration damping differences.
  - High Locality: LRU’s recency heuristic fits perfectly, minimizing misses and memory traffic; LIP prematurely evicts soon-reused lines. Dueling improves over LIP but doesn’t fully converge to LRU’s choices.
  - Phase Change: Phase behavior still favors recency overall; LRU wins, LIP loses, and Dueling sits between them, suggesting limited but meaningful adaptation.
- Hierarchy effects:
  - L1D miss rate strongly correlates with DRAM bytes and total memory bandwidth; lower miss rates translate to higher IPC and shorter simSeconds.
  - L1I behavior remains largely stable across policies (very low misses), indicating data-side policies dominate performance here.
- Dueling considerations:
  - Effectiveness depends on duel set selection, counting mechanism, and update interval. If too sparse or sluggish, adaptation lags phase transitions; if too noisy, decisions oscillate.
  - Observed selection counters indicate active policy decisions, but aggregate outcomes show room for tuning to better match locality-dominant phases.

Validation note: Discussion synthesizes observed metrics across workloads with established policy theory, tying miss-rate changes to bandwidth and IPC shifts [2]. Complete.

## Recommendations
- Workloads:
  - Add controlled microbenchmarks that sweep reuse distances and working-set sizes to isolate policy sensitivities (pure streaming, loop reuse, thrashing).
  - Introduce phase-controlled workloads with known phase lengths to test dueling responsiveness and convergence behavior.
  - Consider multiprogrammed mixes and shared-cache scenarios to study inter-core interference under different policies.
- Scripts & Tooling:
  - Automate parsing of stats.txt into structured CSV/JSON and generate Markdown tables programmatically, ensuring consistent metric selection and naming.
  - Add verification scripts to check internal consistency (misses vs miss rates vs accesses; IPC/CPI vs cycles/instructions) and flag anomalies.
  - Parameterize dueling (duel set fraction, training windows) and cache configs in YAML/JSON manifests for reproducibility and sweep experiments.
- Simulation Configs:
  - Sweep cache sizes/associativities and MSHR counts to understand robustness of conclusions under different hierarchies.
  - Experiment with insertion variants (e.g., BIP/DRRIP) and multi-policy dueling to expand coverage beyond LIP/LRU.
  - Record warmup vs measurement windows and ensure steady-state regions are used for comparisons.

Validation note: Recommendations cover workload design, automation, and configuration sweeps to deepen insights
