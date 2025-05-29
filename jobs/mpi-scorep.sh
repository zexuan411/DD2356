#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH --job-name mpi-scorep
#SBATCH -p shared
#SBATCH --nodes 4
#SBATCH --ntasks-per-node=1
##SBATCH --ntasks 1
#SBATCH --cpus-per-task=64
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

rm build/2-mpi.o
module purge
module load PDC/23.12 
module load score-p/8.4-cpeGNU
scorep mpicc -DNX=12000 -DNSTEPS=10000 -DDEBUG_OUTPUT  -lm -fopenmp -o build/2-mpi.o src/2-mpi.c build/ioutils.o

export OMP_SCHEDULE=static
THREAD_PER_TASK=16
srun --ntasks=16 --cpus-per-task=16 build/2-mpi.o

