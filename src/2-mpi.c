#include "../include/ioutils.h"
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ENABLE_IO
#define ENABLE_IO
#endif

#ifndef DEBUG_OUTPUT
// #define DEBUG_OUTPUT
#endif

#ifndef NX
#define NX 12000 // Total number of spatial points // originally 400
#endif

#ifndef NSTEPS
#define NSTEPS 10000 // Number of time steps
#endif

#define DX 1.0 // Spatial step size
#define DT 0.5 // Time step size (should satisfy the CFL condition)
#define PI 3.141592653589793

// Global variable for MPI
int rank, nprocs;

// Domain Decomposition
int local_len, start_offset;
static inline int stripe_len(int rank, int nprocs) {
  int n_elem = NX / nprocs;
  int remainder = NX % nprocs;
  return (rank < remainder) ? n_elem + 1 : n_elem;
}

static inline int stripe_start(int rank, int nprocs) {
  int n_elem = NX / nprocs;
  int remainder = (NX % nprocs);
  return (rank < remainder) ? (n_elem + 1) * rank
                            : n_elem * rank + (NX % nprocs);
}

// Function to initialize the electric field with a Gaussian pulse
void initialize_fields(double *E, double *H) {
  // Center the Gaussian pulse in the middle of the domain
  double center = NX * DX / 2.0;

  for (int i = 0; i < local_len; i++) {
    double x = (i + start_offset) * DX;
    E[i] = exp(-0.005 * (x - center) * (x - center));

    // hyl: in every process, a memset is enough for initializing H[]
    // H[i] = 0.0;
  }
  memset(H, 0, sizeof(double) * local_len);
}

// Halo Exchange
void halo_exchange(double *buf, int step) {
  // left exchange
  if (rank != 0) {
    double *first_elem_ptr = buf;
    double *left_neighbor_halo_ptr = buf - 1;
    MPI_Sendrecv(first_elem_ptr, 1, MPI_DOUBLE, rank - 1, step,
                 left_neighbor_halo_ptr, 1, MPI_DOUBLE, rank - 1, step,
                 MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
  }

  // right exchange
  if (rank != (nprocs - 1)) {
    double *last_elem_ptr = buf + local_len - 1;
    double *right_neighbor_halo_ptr = last_elem_ptr + 1;
    MPI_Sendrecv(last_elem_ptr, 1, MPI_DOUBLE, rank + 1, step,
                 right_neighbor_halo_ptr, 1, MPI_DOUBLE, rank + 1, step,
                 MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
  }
}

double *gathering_E(double *E_local, double *E) {
  int *displs = calloc(nprocs, sizeof(int));
  int *recvcounts = calloc(nprocs, sizeof(int));
  for (int r = 0; r < nprocs; r++) {
    displs[r] = stripe_start(r, nprocs);
    recvcounts[r] = stripe_len(r, nprocs);
  }
  MPI_Gatherv(E_local, local_len, MPI_DOUBLE, E, recvcounts, displs, MPI_DOUBLE,
              0, MPI_COMM_WORLD);
  return E;
}

// Function to update the magnetic field H
void update_H(double *E, double *H) {
  // Update H from 0 to NX-2 (using forward differences)
#pragma omp parallel for schedule(runtime)
  for (int i = 0; i < local_len - 1; i++) {
    H[i] = H[i] + (DT / DX) * (E[i + 1] - E[i]);
  }

  if (rank == (nprocs - 1)) {
    H[local_len - 1] = H[local_len - 2];
  } else {
    int i = local_len - 1;
    H[i] = H[i] + (DT / DX) * (E[i + 1] - E[i]);
  }
}

// Function to update the electric field E
void update_E(double *E, double *H) {
  // Update E from 1 to NX-1 (using backward differences)
#pragma omp parallel for schedule(runtime)
  for (int i = 1; i < local_len; i++) {
    E[i] = E[i] + (DT / DX) * (H[i] - H[i - 1]);
  }
  if (rank == 0) {
    E[0] = E[1];
  } else {
    int i = 0;
    E[i] = E[i] + (DT / DX) * (H[i] - H[i - 1]);
  }
}

int main(int argc, char **argv) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

#ifdef DEBUG_OUTPUT
  if (rank == 0) {
    int omp_num_threads = omp_get_num_threads();
    printf("omp_num_threads: %d\n", omp_num_threads);
    printf("MPI Number of Processes: %d\n", nprocs);
  }
#endif

  // Allocate fields
  local_len = stripe_len(rank, nprocs);
  start_offset = stripe_start(rank, nprocs);

  // Two additional element for left/right neighbor halo
  double *E_local = (double *)malloc((local_len + 2) * sizeof(double));
  double *H_local = (double *)malloc((local_len + 2) * sizeof(double));
#ifdef ENABLE_IO
  double *E = NULL;
  if (rank == 0)
    E = calloc(NX, sizeof(double));
#endif

  if (E_local == NULL || H_local == NULL) {
    fprintf(stderr, "Memory allocation failed in rank: %d\n", rank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // The first element is for holding left neighbor's halo
  // shift the start ptr to the right
  E_local += 1;
  H_local += 1;

#ifdef ENABLE_IO
  char *log_path_prefix = NULL;
  if (rank == 0) {
    char *log_path_prefix_raw = getenv("LOG_PATH_PREFIX");
    log_path_prefix = calloc(120, sizeof(char));
    if (log_path_prefix_raw != NULL) {
      strcpy(log_path_prefix, log_path_prefix_raw);
    } else {
      log_path_prefix[0] = '\0';
    }
  }
#endif

  // Initialize fields
  initialize_fields(E_local, H_local);

#ifdef DEBUG_OUTPUT
  if (rank == 0)
    printf("initialization finished\n");
#endif

#ifdef ENABLE_IO
  char *fname_buf = NULL;
  gathering_E(E_local, E);
  if (rank == 0) {
    fname_buf = calloc(150, sizeof(char));
    snprintf(fname_buf, 150, "%sE_step0.txt", log_path_prefix);
    save_field_to_file(fname_buf, E, NX);
  }
#endif

  // Main FDTD loop
  double start = MPI_Wtime();
  for (int t = 0; t < NSTEPS; t++) {
    halo_exchange(E_local, t);
    update_H(E_local, H_local);
    halo_exchange(H_local, t);
    update_E(E_local, H_local);

#ifdef ENABLE_IO
    if (t == NSTEPS / 2 - 1) {
      gathering_E(E_local, E);
      if (rank == 0) {
        snprintf(fname_buf, 150, "%sE_step%d.txt", log_path_prefix, t);
        save_field_to_file(fname_buf, E, NX);
      }
    }
#endif
  }
  double end = MPI_Wtime();
  if (rank == 0)
    printf("computing time: %f\n", (end - start));

#ifdef ENABLE_IO
  gathering_E(E_local, E);
  if (rank == 0) {
    snprintf(fname_buf, 150, "%sE_step%d.txt", log_path_prefix, NSTEPS);
    save_field_to_file(fname_buf, E, NX);
    free(E);
  }
#endif

  MPI_Finalize();

  free(E_local - 1);
  free(H_local - 1);
  return 0;
}