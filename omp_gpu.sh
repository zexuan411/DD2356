#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH -p gpu
#SBATCH --cpus-per-task=16
#SBATCH -N 1
#SBATCH -e error
#SBATCH -o output_gpu
#SBATCH --mail-user=czexuan@kth.se
#SBATCH --mail-type=START,END,FAIL

module load rocm
amdclang -O3 -fopenmp -fopenmp-targets=amdgcn-amdhsa \
  -Xopenmp-target=amdgcn-amdhsa -march=gfx90a \
  -o omp_gpu omp_gpu.c -lm

NUM_RUNS=5
total_time=0

for i in $(seq 1 $NUM_RUNS)
do
  echo "Run #$i"
  output=$(srun --ntasks=1 --gpus=1 ./omp_gpu)
  echo "$output"
  time_val=$(echo "$output" | grep "Simulation time" | awk '{print $3}')
  total_time=$(echo "$total_time + $time_val" | bc)
done

avg_time=$(echo "scale=6; $total_time / $NUM_RUNS" | bc)
echo "Average GPU simulation time over $NUM_RUNS runs: $avg_time seconds"


# #profiling
# #!/bin/bash
# #SBATCH -A edu25.dd2356
# #SBATCH --time=00:10:00
# #SBATCH -p gpu
# #SBATCH -N 1
# #SBATCH -e error_gpu
# #SBATCH -o output_gpu
# #SBATCH --gpus=1
# #SBATCH --mail-user=czexuan@kth.se
# #SBATCH --mail-type=START,END,FAIL

# module load rocm
# amdclang -O3 -fopenmp -fopenmp-targets=amdgcn-amdhsa \
#   -Xopenmp-target=amdgcn-amdhsa -march=gfx90a \
#   -o omp_gpu omp_gpu.c -lm
# rocprof --hsa-trace --timestamp on --stats ./omp_gpu > gpu_profile.out

