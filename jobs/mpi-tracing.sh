#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH --job-name mpi-tracing
#SBATCH -p shared
#SBATCH --nodes 1
##SBATCH --ntasks-per-node=1
#SBATCH --ntasks 16
#SBATCH --cpus-per-task=1
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

rm build/2-mpi.o
module purge
module load PDC/23.12 
module load score-p/8.4-cpeGNU
scorep mpicc -DNX=12000 -DNSTEPS=10000 -DDEBUG_OUTPUT  -lm -fopenmp -o build/2-mpi.o src/2-mpi.c build/ioutils.o

export OMP_SCHEDULE=static
THREAD_PER_TASK=1

# enable tracing
export SCOREP_ENABLE_PROFILING=false
export SCOREP_ENABLE_TRACING=true
srun --ntasks=16 --cpus-per-task=1 build/2-mpi.o

