import numpy as np
import matplotlib.pyplot as plt
from toolz.curried import *
import argparse
import glob
from pathlib import Path
import re
from typing import Optional
import time

parser = argparse.ArgumentParser()
parser.add_argument("dir", default=".", help="the path of output directory")
parser.add_argument(
    "--nx", default=12000, help="the number of spatial points", type=float
)
parser.add_argument("--dx", default=1, help="spatial step size", type=float)
args = parser.parse_args()


def extract_step_from_fname(fname: str) -> Optional[int]:
    log_name_pattern = r"E_step(\d+).txt"
    m = re.search(log_name_pattern, fname)
    if not m:
        return None

    try:
        return int(m.group(1))
    except ValueError:
        return None


datas = pipe(
    glob.glob(f"{args.dir}/E_*.txt"),
    map(lambda p: (extract_step_from_fname(p), np.loadtxt(Path(p)))),
    filter(lambda t: t[0] != None),
    list,
)


fig, ax = plt.subplots(1)
for d in datas:
    x = np.linspace(0.0, args.nx * args.dx, len(d[1]))
    ax.plot(x, d[1], label=d[0])
ax.legend()
fig.savefig(f"figures/validation-{int(time.time())}.png", dpi=150)
plt.show()
