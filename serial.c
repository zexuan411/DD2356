#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// #define NX 1200         // Total number of spatial points // originally 400
// #define NSTEPS 1000   // Number of time steps
#define NX 12000         // Total number of spatial points // originally 400
#define NSTEPS 10000   // Number of time steps
#define DX 1.0         // Spatial step size
#define DT 0.5         // Time step size (should satisfy the CFL condition)
#define PI 3.141592653589793

// Function to initialize the electric field with a Gaussian pulse
void initialize_fields(double *E, double *H) {
    // Center the Gaussian pulse in the middle of the domain
    double center = NX * DX / 2.0;
    for (int i = 0; i < NX; i++) {
        double x = i * DX;
        double val = exp(-0.005 * (x - center) * (x - center));
        E[i] = exp(-0.005 * (x - center) * (x - center));
        H[i] = 0.0;
    }
}

// Function to update the magnetic field H
void update_H(double *E, double *H) {
    // Update H from 0 to NX-2 (using forward differences)
    for (int i = 0; i < NX - 1; i++) {
        H[i] = H[i] + (DT / DX) * (E[i + 1] - E[i]);
    }
    // Simple absorbing boundary condition:
    H[NX - 1] = H[NX - 2];
}

// Function to update the electric field E
void update_E(double *E, double *H) {
    // Update E from 1 to NX-1 (using backward differences)
    for (int i = 1; i < NX; i++) {
        E[i] = E[i] + (DT / DX) * (H[i] - H[i - 1]);
    }
    // Simple absorbing boundary condition:
    E[0] = E[1];
}

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
    // Allocate fields
    double *E = (double *) malloc(NX * sizeof(double));
    double *H = (double *) malloc(NX * sizeof(double));
    if (E == NULL || H == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
        
    // Initialize fields
    initialize_fields(E, H);
    save_field_to_file("E_step0.txt", E, NX);
    
    // test
    // printf("Electric field near center:\n");
    // for (int i = 180; i <= 220; i++) {
    //     printf("%f ", E[i]);
    // }
    // printf("\n");
    
    int mid_step = NSTEPS / 2;


    clock_t start_time = clock();

    // Main FDTD loop
    for (int t = 0; t < NSTEPS; t++) {
        update_H(E, H);
        update_E(E, H);
        if (t == mid_step - 1) {
            char filename[50];
            sprintf(filename, "E_step%d.txt", mid_step);
            save_field_to_file(filename, E, NX);
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Computation time: %.6f seconds\n", elapsed_time);

    // test
    // printf("Electric field near center:\n");
    // for (int i = 180; i <= 220; i++) {
    //     printf("%f ", E[i]);
    // }
    // printf("\n");

    char filename_end[50];
    sprintf(filename_end, "E_step%d.txt", NSTEPS);
    save_field_to_file(filename_end, E, NX);


    free(E);
    free(H);
    return 0;
}