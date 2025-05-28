#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH --job-name mpi-scaling
#SBATCH -p shared
#SBATCH --nodes 4
#SBATCH --ntasks-per-node=1
##SBATCH --ntasks 2
#SBATCH --cpus-per-task=64
#SBATCH -o logs/%x-%j.log
#SBATCH --mail-user=hengyil@kth.se
#SBATCH --mail-type=START,END,FAIL

rm build/2-mpi.o
make build/2-mpi.o CFLAGS="-DNX=12000 -DNSTEPS=10000 -DDEBUG_OUTPUT"

export OMP_SCHEDULE=static
# strong scaling
THREAD_PER_TASK=16
for i in $(seq 1 16); do
    export OMP_NUM_THREADS=${THREAD_PER_TASK}
    echo "--ntasks-per-node=${i}, --cpus-per-task=${THREAD_PER_TASK}"
    for t in $(seq 1 5); do
        echo "task: ${t}"
        srun --ntasks="${i}" --cpus-per-task="${THREAD_PER_TASK}" build/2-mpi.o
        echo "task: ${t} done"
    done
    echo "-----"
done

echo "====="

# weak scaling
THREAD_PER_TASK=16
for i in $(seq 1 16); do
    INPUT_SIZE=$(bc <<<"12000 * ${i} * ${THREAD_PER_TASK}")

    rm build/2-mpi.o
    make build/2-mpi.o CFLAGS="-DNX=${INPUT_SIZE} -DNSTEPS=10000 -DDEBUG_OUTPUT"

    echo "input size: ${INPUT_SIZE}"

    export OMP_NUM_THREADS=${THREAD_PER_TASK}
    echo "--ntasks-per-node=${i}, --cpus-per-task=${THREAD_PER_TASK}"
    for t in $(seq 1 5); do
        echo "task: ${t}"
        srun --ntasks="${i}" --cpus-per-task="${THREAD_PER_TASK}" build/2-mpi.o
        echo "task: ${t} done"
    done
    echo "-----"
done
