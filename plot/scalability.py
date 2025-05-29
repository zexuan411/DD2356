import re
import time
from pathlib import Path
from toolz.curried import *
import numpy as np
import matplotlib.pyplot as plt

LOG_PATH = Path("logs") / "mpi-scaling-10328988.log"

serial = [
    0.805750,
    0.804367,
    0.804561,
    0.806059,
    0.804673,
]
serial_avg = np.mean(serial)

print(f"[serial] avg: {np.mean(serial)}, standard derivation: {np.std(serial)}")


def extract_time_from_log_block(b: str):
    time_log_pattern = re.compile(r"computing time: ([\d\.]+)")
    return pipe(
        re.finditer(time_log_pattern, b),
        filter(identity),
        map(lambda m: float(m[1])),
        list,
    )


with open(LOG_PATH) as f:
    datas = pipe(
        f.read(),
        lambda s: s.split("====="),
        map(lambda s: s.split("-----")),
        map(filter(identity)),
        map(map(extract_time_from_log_block)),
        lambda it: [list(i) for i in it],
    )

strong_measurement = pipe(
    datas[0],
    filter(identity),
    list,
)

weak_measurement = pipe(
    datas[1],
    filter(identity),
    list,
)


def plot_scalability(ax, data, title):
    data = serial_avg * np.reciprocal(data)
    threads = np.linspace(16, 256, 16)
    ax.errorbar(
        threads,
        np.mean(data, axis=1),
        yerr=np.std(data, axis=1),
        capsize=3,
        ecolor="black",
    )
    ax.set_title(title)
    ax.set_xlabel("Thread Number")
    ax.set_ylabel("Speedup")


fig, (ax1, ax2) = plt.subplots(2)
plt.subplots_adjust(hspace=0.5)
plot_scalability(ax1, strong_measurement, "Strong Scaling")
plot_scalability(ax2, weak_measurement, "Weak Scaling")
plt.savefig(f"figures/scalability-{int(time.time())}.png", dpi=150)
plt.show()
