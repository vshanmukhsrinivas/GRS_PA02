import matplotlib.pyplot as plt
import numpy as np

system_info = "Ubuntu 22.04 | 8 Threads | 5 sec run | perf counters"

message_sizes = np.array([1024, 4096, 16384, 65536])
threads = np.array([1, 2, 4, 8])

# DATA (From Your CSV)


A1_thr = np.array([0.003218, 0.012875, 0.051438, 0.206474])
A2_thr = np.array([0.003217, 0.012869, 0.051410, 0.205712])
A3_thr = np.array([0.003217, 0.012877, 0.051399, 0.205840])

A1_lat = np.array([40678.569105, 20351.894308, 10169.876016, 5090.030487])
A2_lat = np.array([40669.715447, 20342.235772, 10170.317073, 5092.091463])
A3_lat = np.array([40710.284552, 20360.252032, 10179.304878, 5092.274390])

A1_llc = np.array([188406, 188490, 292081, 700194])
A2_llc = np.array([186306, 189649, 287471, 701568])
A3_llc = np.array([190904, 195363, 284477, 651992])

A1_cycles = np.array([140808308, 137065998, 143149290, 208729052])
A2_cycles = np.array([139190128, 141621347, 142920966, 203763220])
A3_cycles = np.array([146269477, 137181931, 142075590, 213186065])

# HELPER FUNCTION WITH ZOOM + OFFSET


def save_plot(x, y1, y2, y3, xlabel, ylabel, title, filename):

    plt.figure(figsize=(8,6))

    # small offset so curves don't overlap exactly
    offset = 0.005 * (max(x) - min(x))
    x1 = x - offset
    x2 = x
    x3 = x + offset

    plt.plot(x1, y1, marker='o', linewidth=2, linestyle='--', label="A1 - Two Copy")
    plt.plot(x2, y2, marker='s', linewidth=2, linestyle='-.', label="A2 - One Copy")
    plt.plot(x3, y3, marker='^', linewidth=2, linestyle='-', label="A3 - Zero Copy")

    # Dynamic zoom on Y-axis
    ymin = min(np.min(y1), np.min(y2), np.min(y3))
    ymax = max(np.max(y1), np.max(y2), np.max(y3))
    margin = (ymax - ymin) * 0.15
    plt.ylim(ymin - margin, ymax + margin)

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title + "\n" + system_info)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.close()


# Throughput vs Message Size


save_plot(
    message_sizes,
    A1_thr, A2_thr, A3_thr,
    "Message Size (Bytes)",
    "Throughput (Gbps)",
    "Throughput vs Message Size (8 Threads)",
    "throughput_vs_message_size.png"
)


# Latency vs Thread Count


save_plot(
    threads,
    A1_lat, A2_lat, A3_lat,
    "Thread Count",
    "Latency (µs)",
    "Latency vs Thread Count (1024 Bytes)",
    "latency_vs_threads.png"
)


# LLC Cache Misses vs Message Size


save_plot(
    message_sizes,
    A1_llc, A2_llc, A3_llc,
    "Message Size (Bytes)",
    "LLC Cache Misses",
    "LLC Cache Misses vs Message Size (8 Threads)",
    "cache_misses_vs_message_size.png"
)


# CPU Cycles per Byte


def cycles_per_byte(cycles, throughput):
    bytes_transferred = (throughput * 1e9 * 5) / 8
    return cycles / bytes_transferred

A1_cpb = cycles_per_byte(A1_cycles, A1_thr)
A2_cpb = cycles_per_byte(A2_cycles, A2_thr)
A3_cpb = cycles_per_byte(A3_cycles, A3_thr)

save_plot(
    message_sizes,
    A1_cpb, A2_cpb, A3_cpb,
    "Message Size (Bytes)",
    "CPU Cycles per Byte",
    "CPU Cycles per Byte Transferred (8 Threads)",
    "cycles_per_byte.png"
)

print("All 4 improved plots generated successfully.")
