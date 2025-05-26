#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:05:00
#SBATCH --job-name mpi-validate
#SBATCH -p shared
##SBATCH --nodes 1
##SBATCH --ntasks-per-node=2
#SBATCH --ntasks 2
#SBATCH --cpus-per-task=16
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

set -x
export OMP_NUM_THREADS=16
srun build/2-mpi.o
