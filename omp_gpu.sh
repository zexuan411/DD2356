#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH -p gpu
#SBATCH -N 1                   
#SBATCH -e error
#SBATCH -o output_gpu
#SBATCH --mail-user=czexuan@kth.se
#SBATCH --mail-type=START,END,FAIL

module load rocm
amdclang -fopenmp -fopenmp-targets=amdgcn-amdhsa \
  -Xopenmp-target=amdgcn-amdhsa -march=gfx90a \
  -o omp_gpu omp_gpu.c -lm
# srun --ntasks=1 ./omp_gpu 
rocprof --hsa-trace --timestamp on -o trace_output.csv ./omp_gpu
