#include "../include/ioutils.h"
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define ENABLE_IO

#ifndef NX
#define NX 12000 // Total number of spatial points // originally 400
#endif

#ifndef NSTEPS
#define NSTEPS 10000 // Number of time steps
#endif

#define DX 1.0 // Spatial step size
#define DT 0.5 // Time step size (should satisfy the CFL condition)
#define PI 3.141592653589793

// Function to initialize the electric field with a Gaussian pulse
void initialize_fields(double *E, double *H) {
  // Center the Gaussian pulse in the middle of the domain
  double center = NX * DX / 2.0;
  for (int i = 0; i < NX; i++) {
    double x = i * DX;
    E[i] = exp(-0.005 * (x - center) * (x - center));
    H[i] = 0.0;
  }
}

// Function to update the magnetic field H
void update_H(double *E, double *H) {
#pragma omp parallel for schedule(runtime)
  // Update H from 0 to NX-2 (using forward differences)
  for (int i = 0; i < NX - 1; i++) {
    H[i] = H[i] + (DT / DX) * (E[i + 1] - E[i]);
  }
  // Simple absorbing boundary condition:
  H[NX - 1] = H[NX - 2];
}

// Function to update the electric field E
void update_E(double *E, double *H) {
#pragma omp parallel for schedule(runtime)
  // Update E from 1 to NX-1 (using backward differences)
  for (int i = 1; i < NX; i++) {
    E[i] = E[i] + (DT / DX) * (H[i] - H[i - 1]);
  }
  // Simple absorbing boundary condition:
  E[0] = E[1];
}

int main() {
  // Allocate fields
  double *E = (double *)malloc(NX * sizeof(double));
  double *H = (double *)malloc(NX * sizeof(double));
  if (E == NULL || H == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

#ifdef ENABLE_IO
  char *log_path_prefix_raw = getenv("LOG_PATH_PREFIX");
  char *log_path_prefix = calloc(120, sizeof(char));
  if (log_path_prefix_raw != NULL) {
    strcpy(log_path_prefix, log_path_prefix_raw);
  } else {
    log_path_prefix[0] = '\0';
  }
#endif

  // Initialize fields
  initialize_fields(E, H);

#ifdef ENABLE_IO
  char fname_buf[150];
  snprintf(fname_buf, 150, "%sE_step0.txt", log_path_prefix);
  save_field_to_file(fname_buf, E, NX);
#endif

  // Main FDTD loop
  double start = omp_get_wtime();
  for (int t = 0; t < NSTEPS; t++) {
    update_H(E, H);
    update_E(E, H);

#ifdef ENABLE_IO
    if (t == NSTEPS / 2 - 1) {
      snprintf(fname_buf, 150, "%sE_step%d.txt", log_path_prefix, t);
      save_field_to_file(fname_buf, E, NX);
    }
#endif

  }
  double end = omp_get_wtime();
  printf("computing time: %f\n", (end - start));

#ifdef ENABLE_IO
  snprintf(fname_buf, 150, "%sE_step%d.txt", log_path_prefix, NSTEPS);
  save_field_to_file(fname_buf, E, NX);
#endif

  free(E);
  free(H);
  return 0;
}