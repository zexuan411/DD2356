#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:20:00
#SBATCH --job-name openmp
#SBATCH -p shared
#SBATCH --nodes 1
##SBATCH --ntasks-per-node=2
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=64
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

for sche in static dynamic guided; do
    export OMP_SCHEDULE=${sche}
    for tnum in 16 32 64; do
        export OMP_NUM_THREADS=${tnum}
        echo "schedule:${OMP_SCHEDULE};threads:${OMP_NUM_THREADS}"
        for t in $(seq 1 3); do
            echo "test: ${t}"
            srun build/1-openmp.o
            echo "test: ${t} done"
        done
        echo "-----"
    done
    echo "====="
done
