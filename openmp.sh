#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH -p shared
#SBATCH -N 1                   
#SBATCH -e error
#SBATCH -o output_openmp
#SBATCH --mail-user=czexuan@kth.se
#SBATCH --mail-type=START,END,FAIL

gcc -o openmp openmp.c -fopenmp -lm

threads=(1 2 4 8 16)

for t in "${threads[@]}"
do
    export OMP_NUM_THREADS=$t
    runtime=$(./openmp | grep "Simulation time" | awk '{print $3}')
    echo "$t,$runtime"
done