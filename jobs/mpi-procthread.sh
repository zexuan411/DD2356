#!/bin/bash
#SBATCH -A edu25.dd2356
#SBATCH --time=00:10:00
#SBATCH --job-name mpi-procthread
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

for pnum in 4 8 16 32; do
    tnum=$(bc <<<"256 / ${pnum}")
    pnum_pernode=$(bc <<<"${pnum} / 4")
    echo "--ntasks-per-node=${pnum_pernode}, --cpus-per-task=${tnum}"
    export OMP_NUM_THREADS="${tnum}"

    for t in $(seq 1 3); do
        echo "test: ${t}"
        srun --ntasks=${pnum} --ntasks-per-node="${pnum_pernode}" --cpus-per-task="${tnum}" build/2-mpi.o
        echo "test: ${t}, done"
    done

    echo "-------"
done
