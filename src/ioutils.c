#include <stdio.h>

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
