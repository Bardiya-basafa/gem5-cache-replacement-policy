import argparse
import os
from gem5.resources.resource import obtain_resource
import m5
import m5.objects as o
from m5.objects import (
    System,
    SrcClockDomain,
    VoltageDomain,
    TimingSimpleCPU,
    Cache,
    SystemXBar,
    MemCtrl,
    DDR3_1600_8x8,
    AddrRange,
    Root,
    SEWorkload,
    Process,
    SimpleMemory
)


parser = argparse.ArgumentParser(description="LIP policy SE run (non-stdlib)")
parser.add_argument(
    "--workload",
    default="streaming",
    choices=[
        "streaming",
        "streaming_light",
        "high_locality",
        "high_locality_light",
        "phase_change",
        "phase_change_light",
        "hello",  # uses gem5 resource x86-hello64-static
    ],
    help="Which workload binary to run (default: streaming).",
)
parser.add_argument(
    "workload_args",
    nargs=argparse.REMAINDER,
    help="Arguments passed to the workload; use -- to separate.",
)
args = parser.parse_args()

# Remove a leading '--' in remainder if present
wl_args = args.workload_args
if wl_args and wl_args[0] == "--":
    wl_args = wl_args[1:]

# Map workload name to a binary path (assumes binaries are in CWD)
workload_map = {
    "streaming": "./streaming",
    "streaming_light": "./streaming_light",
    "high_locality": "./high_locality",
    "high_locality_light": "./high_locality_light",
    "phase_change": "./phase_change",
    "phase_change_light": "./phase_change_light",
}

if args.workload == "hello":
    binary = obtain_resource("x86-hello64-static").get_local_path()
else:
    binary = workload_map[args.workload]
    if not os.path.exists(binary):
        raise FileNotFoundError(
            f"Workload binary not found: {binary}. Build it or adjust path."
        )


system = System()
system.clk_domain = SrcClockDomain(
    clock="3GHz", voltage_domain=VoltageDomain()
)
system.mem_mode = "timing"
system.cache_line_size = 64
system.mem_ranges = [AddrRange("1GB")]

system.cpu = TimingSimpleCPU()
system.membus = SystemXBar()

lip1d = o.LIPRP()
lip1d.btp = 0
lip1i = o.LIPRP()
lip1i.btp = 0


system.cpu.icache = Cache(
    size="32kB",
    assoc=4,
    tag_latency=1,
    data_latency=1,
    response_latency=1,
    mshrs=8,
    tgts_per_mshr=20,
    replacement_policy=lip1i,
)
system.cpu.dcache = Cache(
    size="32kB",
    assoc=4,
    tag_latency=1,
    data_latency=1,
    response_latency=1,
    mshrs=8,
    tgts_per_mshr=20,
    replacement_policy=lip1d,
)

# Connect L1s
system.cpu.icache.cpu_side = system.cpu.icache_port
system.cpu.dcache.cpu_side = system.cpu.dcache_port

# No L2: connect directly to membus
system.cpu.icache.mem_side = system.membus.slave
system.cpu.dcache.mem_side = system.membus.slave

# Fixed-latency main memory (simplifies comparisons)
system.mem = SimpleMemory(range=system.mem_ranges[0], latency="50ns", bandwidth="16GB/s")
system.mem.port = system.membus.master



# system.mem_ctrl = MemCtrl()
# system.mem_ctrl.dram = DDR3_1600_8x8(range=system.mem_ranges[0])
# system.mem_ctrl.port = system.membus.master

system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.master
system.cpu.interrupts[0].int_slave = system.membus.master
system.cpu.interrupts[0].int_master = system.membus.slave
system.system_port = system.membus.slave

# binary = "streaming"

# for gem5 V21 and beyond
system.workload = SEWorkload.init_compatible(binary)

p = Process()
p.cmd = [binary]
system.cpu.workload = p
system.cpu.createThreads()

root = Root(full_system=False, system=system)
m5.instantiate()
ev = m5.simulate()
print(f"Exited at tick {m5.curTick()} because {ev.getCause()}")
