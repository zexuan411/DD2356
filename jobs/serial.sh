#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:05:00
#SBATCH --job-name serial
#SBATCH -p shared
#SBATCH --nodes 1
##SBATCH --ntasks-per-node=2
#SBATCH --ntasks 1
#SBATCH --cpus-per-task=1
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

for t in $(seq 1 5); do
    srun build/0-serial.o
done
