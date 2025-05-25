#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define NX 12000
#define NSTEPS 10000
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


int main() {
    double *E = (double *) malloc(NX * sizeof(double));
    double *H = (double *) malloc(NX * sizeof(double));
    if (E == NULL || H == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    initialize_fields(E, H);

    // Offload data to GPU once
    #pragma omp target enter data map(to: E[0:NX], H[0:NX])

    double start_time = omp_get_wtime();

    #pragma omp target teams distribute parallel for
    for (int i = 0; i < NX; i++) {
        if (i < NX - 1)
            H[i] += (DT / DX) * (E[i + 1] - E[i]);
        if (i == NX - 1)
            H[i] = H[NX - 2];
    }

    #pragma omp target teams distribute parallel for
    for (int i = 0; i < NX; i++) {
        if (i > 0)
            E[i] += (DT / DX) * (H[i] - H[i - 1]);
        if (i == 0)
            E[i] = E[1];
    }

    double end_time = omp_get_wtime();
    printf("Simulation time: %.6f seconds\n", end_time - start_time);

    // Bring results back to host
    #pragma omp target exit data map(from: E[0:NX], H[0:NX])

    free(E);
    free(H);
    return 0;
}
