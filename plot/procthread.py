import re
from pathlib import Path
from toolz.curried import *
import numpy as np
import matplotlib.pyplot as plt
import time

LOG_PATH = Path("logs") / "mpi-procthread-10302628.log"


def extract_time_from_log_block(b: str):
    time_log_pattern = re.compile(r"computing time: ([\d\.]+)")
    return pipe(
        re.finditer(time_log_pattern, b),
        filter(identity),
        map(
            lambda m: float(m[1]),
        ),
        list,
    )


with open(LOG_PATH) as f:
    datas = pipe(
        f.read(),
        lambda s: s.split("-------"),
        map(lambda s: s.strip()),
        filter(identity),
        map(extract_time_from_log_block),
        list,
    )

thread_over_procs = [64 / 4, 32 / 8, 16 / 16, 8 / 32]

plt.errorbar(
    thread_over_procs,
    np.mean(np.array(datas), axis=1),
    capsize=3,
    yerr=np.std(np.array(datas)),
    fmt="r--o",
    ecolor="black",
)
plt.xlabel("Thread Number/Process Number")
plt.ylabel("Execution Time (seconds)")
plt.savefig(f"figures/threadprocs-{int(time.time())}.png", dpi=150)
plt.show()
