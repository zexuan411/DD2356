#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>  

#define NX 12000      // Total number of spatial points
#define NSTEPS 10000    // Number of time steps
#define DX 1.0          // Spatial step size
#define DT 0.5          // Time step size (should satisfy the CFL condition)
#define PI 3.141592653589793

// Function to initialize the electric field with a Gaussian pulse
void initialize_fields(double *E, double *H) {
    double center = NX * DX / 2.0;
    for (int i = 0; i < NX; i++) {
        double x = i * DX;
        E[i] = exp(-0.005 * (x - center) * (x - center));
        H[i] = 0.0;
    }
}

// Function to update the magnetic field H
void update_H(double *E, double *H) {
    #pragma omp target teams distribute parallel for 
    for (int i = 0; i < NX - 1; i++) {
        H[i] += (DT / DX) * (E[i + 1] - E[i]);
    }
    // H[NX - 1] = H[NX - 2];  // Boundary condition (not parallelized)
    #pragma omp target
    {
        H[NX - 1] = H[NX - 2];
    }
}

// Function to update the electric field E
void update_E(double *E, double *H) {
    #pragma omp target teams distribute parallel for
    for (int i = 1; i < NX; i++) {
        E[i] += (DT / DX) * (H[i] - H[i - 1]);
    }
    #pragma omp target
    {
        E[0] = E[1];
    }   
    // E[0] = E[1];  // Boundary condition (not parallelized)
}

// Save electric field to file
void save_field_to_file(const char* filename, double *E, int length) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        return;
    }
    for (int i = 0; i < length; i++) {
        fprintf(fp, "%.10f\n", E[i]);
    }
    fclose(fp);
}

int main() {
    double *E = (double *) malloc(NX * sizeof(double));
    double *H = (double *) malloc(NX * sizeof(double));
    if (E == NULL || H == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    initialize_fields(E, H);
    // save_field_to_file("E_step0.txt", E, NX);
    int mid_step = NSTEPS / 2;

    #pragma omp target enter data map(to: E[0:NX], H[0:NX])
    // Start timing
    double start_time = omp_get_wtime();

    for (int t = 0; t < NSTEPS; t++) {
        update_H(E, H);
        update_E(E, H);
        // if (t == mid_step - 1) {
        //     char filename[50];
        //     sprintf(filename, "E_step%d.txt", mid_step);
        //     save_field_to_file(filename, E, NX);
        // }
    }
    
    // End timing
    double end_time = omp_get_wtime();
    printf("Simulation time: %.6f seconds\n", end_time - start_time);

    #pragma omp target exit data map(from: E[0:NX], H[0:NX])


    // Save final field
    // char filename_end[50];
    // sprintf(filename_end, "E_step%d.txt", NSTEPS);
    // save_field_to_file(filename_end, E, NX);

    free(E);
    free(H);
    return 0;
}
