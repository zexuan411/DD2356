
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define NX 120000
#define NSTEPS 100000
#define DX 1.0
#define DT 0.5
#define PI 3.141592653589793

// Function to initialize the electric and magnetic fields
void initialize_fields(double *E, double *H) {
    double center = NX * DX / 2.0;
    for (int i = 0; i < NX; i++) {
        double x = i * DX;
        E[i] = exp(-0.005 * (x - center) * (x - center));
        H[i] = 0.0;
    }
}

// Function to save field data to file
void save_field_to_file(const char* filename, double *field, int length) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        return;
    }
    for (int i = 0; i < length; i++) {
        fprintf(fp, "%.10f\n", field[i]);
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
    
    // Save initial field
    // save_field_to_file("E_step0.txt", E, NX);

    int mid_step = NSTEPS / 2;

    // Offload data to GPU
    #pragma omp target enter data map(to: E[0:NX], H[0:NX])

    double start_time = omp_get_wtime();

    for (int t = 0; t < NSTEPS; t++) {
        // Update magnetic field H
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < NX; i++) {
            if (i < NX - 1)
                H[i] += (DT / DX) * (E[i + 1] - E[i]);
            else
                H[i] = H[NX - 2];  // Right boundary condition
        }

        // Update electric field E
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < NX; i++) {
            if (i > 0)
                E[i] += (DT / DX) * (H[i] - H[i - 1]);
            else
                E[i] = E[1];  // Left boundary condition
        }

        // Save mid-step field 
        // if (t == mid_step - 1) {
        //     #pragma omp target update from(E[0:NX])
        //     char filename[50];
        //     sprintf(filename, "E_step%d.txt", mid_step);
        //     save_field_to_file(filename, E, NX);
        //     #pragma omp target update to(E[0:NX])
        // }
    }

    double end_time = omp_get_wtime();
    printf("Simulation time: %.6f seconds\n", end_time - start_time);

    // Bring final results back to host
    #pragma omp target exit data map(from: E[0:NX], H[0:NX])

    // Save final field
    // char filename_end[50];
    // sprintf(filename_end, "E_step%d.txt", NSTEPS);
    // save_field_to_file(filename_end, E, NX);

    free(E);
    free(H);
    return 0;
}
