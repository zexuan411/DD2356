// MPI+OpenMP

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define NX 12000
#define NSTEPS 10000
#define DX 1.0
#define DT 0.5

void initialize_fields(double *E, double *H, int local_N, int rank, int size) {
    double center_global = NX * DX / 2.0;
    int start_index = rank * local_N;

    for (int i = 0; i < local_N; i++) {
        double x = (start_index + i) * DX;
        E[i] = exp(-0.005 * (x - center_global) * (x - center_global));
        H[i] = 0.0;
    }
}

void save_field_to_file(const char* filename, double *E_global) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        return;
    }
    for (int i = 0; i < NX; i++) {
        fprintf(fp, "%.10f\n", E_global[i]);
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_N = NX / size;

    double *E_local_raw = (double *) malloc((local_N + 3) * sizeof(double));
    double *H_local_raw = (double *) malloc((local_N + 3) * sizeof(double));
    if (!E_local_raw || !H_local_raw) {
        fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double *E_local = E_local_raw + 1;
    double *H_local = H_local_raw + 1;

    initialize_fields(E_local, H_local, local_N, rank, size);

    int left = (rank == 0) ? MPI_PROC_NULL : rank - 1;
    int right = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

    MPI_Request reqs[4];
    MPI_Status stats[4];

    double start_time = MPI_Wtime();

    for (int t = 0; t < NSTEPS; t++) {
        MPI_Isend(&E_local[local_N], 1, MPI_DOUBLE, right, 0, MPI_COMM_WORLD, &reqs[0]);
        MPI_Irecv(&E_local[0], 1, MPI_DOUBLE, left, 0, MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(&E_local[1], 1, MPI_DOUBLE, left, 1, MPI_COMM_WORLD, &reqs[2]);
        MPI_Irecv(&E_local[local_N + 1], 1, MPI_DOUBLE, right, 1, MPI_COMM_WORLD, &reqs[3]);

        MPI_Waitall(4, reqs, stats);

        for (int i = 1; i <= local_N - 1; i++) {
            H_local[i] = H_local[i] + (DT / DX) * (E_local[i + 1] - E_local[i]);
        }
        if (right == MPI_PROC_NULL) {
            H_local[local_N] = H_local[local_N - 1];
        } else {
            H_local[local_N] = H_local[local_N] + (DT / DX) * (E_local[local_N + 1] - E_local[local_N]);
        }

        MPI_Isend(&H_local[local_N], 1, MPI_DOUBLE, right, 2, MPI_COMM_WORLD, &reqs[0]);
        MPI_Irecv(&H_local[0], 1, MPI_DOUBLE, left, 2, MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(&H_local[1], 1, MPI_DOUBLE, left, 3, MPI_COMM_WORLD, &reqs[2]);
        MPI_Irecv(&H_local[local_N + 1], 1, MPI_DOUBLE, right, 3, MPI_COMM_WORLD, &reqs[3]);

        MPI_Waitall(4, reqs, stats);

        for (int i = 2; i <= local_N; i++) {
            E_local[i] = E_local[i] + (DT / DX) * (H_local[i] - H_local[i - 1]);
        }
        if (left == MPI_PROC_NULL) {
            E_local[1] = E_local[2];
        } else {
            E_local[1] = E_local[1] + (DT / DX) * (H_local[1] - H_local[0]);
        }
    }

    E_local--;

    double *E_global = NULL;
    if (rank == 0) {
        E_global = (double *) malloc(NX * sizeof(double));
    }

    MPI_Gather(E_local + 1, local_N, MPI_DOUBLE,
               E_global, local_N, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    if(rank == 0) {
        printf("Total time taken: %f seconds\n", end_time - start_time);
        fflush(stdout);
    }

    if (rank == 0) {
        // save_field_to_file("E_final.txt", E_global);
        free(E_global);
    }

    free(E_local_raw);
    free(H_local_raw);

    MPI_Finalize();
    return 0;
}
