#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:05:00
#SBATCH -p shared
#SBATCH -N 1                  
#SBATCH --ntasks-per-node=16    
#SBATCH -e error
#SBATCH -o output_mpi
#SBATCH --mail-user=czexuan@kth.se
#SBATCH --mail-type=START,END,FAIL

EXE=./mpi
PROCS=(1 2 4 8 16)    

for p in "${PROCS[@]}"
do
    echo "Running with $p process(es)..."
    srun -n $p $EXE
done