#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 1024
#define MAX_DATA 100000

int main() {
    char filename[256];
    char line[MAX_LINE];

    printf("Input filename: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\r\n")] = 0;

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open input file");
        return 1;
    }
    printf("Opening file: %s\n", filename);

    // 分配記憶體
    int *x = malloc(MAX_DATA * sizeof(int));
    int *y = malloc(MAX_DATA * sizeof(int));
    int *z = malloc(MAX_DATA * sizeof(int));
    int *num = malloc(MAX_DATA * sizeof(int));
    float *mag_x = malloc(MAX_DATA * sizeof(float));
    float *mag_y = malloc(MAX_DATA * sizeof(float));
    float *mag_z = malloc(MAX_DATA * sizeof(float));
    float *magnitude = malloc(MAX_DATA * sizeof(float));
    float *heading = malloc(MAX_DATA * sizeof(float));
    bool *used = calloc(MAX_DATA, sizeof(bool));

    int i = 0;

    // 跳過標頭
    if (!fgets(line, sizeof(line), file)) {
        printf("File is empty!\n");
        return 1;
    }

    // 讀取 CSV
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ","); // Time,跳過
        if (!token) continue;

        token = strtok(NULL, ","); if (!token) continue; x[i] = atoi(token);
        token = strtok(NULL, ","); if (!token) continue; y[i] = atoi(token);
        token = strtok(NULL, ","); if (!token) continue; z[i] = atoi(token);
        token = strtok(NULL, ","); if (!token) continue; num[i] = atoi(token);
        token = strtok(NULL, ","); if (!token) continue; mag_x[i] = atof(token);
        token = strtok(NULL, ","); if (!token) continue; mag_y[i] = atof(token);
        token = strtok(NULL, ","); if (!token) continue; mag_z[i] = atof(token);
        token = strtok(NULL, ","); if (!token) continue; magnitude[i] = atof(token);
        token = strtok(NULL, ","); if (!token) continue; heading[i] = atof(token);

        i++;
        if (i >= MAX_DATA) {
            printf("Reached MAX_DATA limit!\n");
            break;
        }
    }

    printf("Read %d valid records from CSV\n", i);
    fclose(file);

    // 輸出 result.csv
    FILE *out = fopen("result.csv", "w");
    if (!out) {
        perror("Cannot open output file");
        return 1;
    }
    fprintf(out, "x,y,z,Mag_X,Mag_Y,Mag_Z,magnitude,heading,count\n");

    // 分組平均
    for (int k = 0; k < i; k++) {
        if (used[k]) continue;
        used[k] = true;
        int xi = x[k], yi = y[k], zi = z[k];
        float sum_magx = 0, sum_magy = 0, sum_magz = 0;
        float sum_magnitude = 0, sum_heading = 0;
        int count = 0;

        for (int j = k; j < i; j++) {
            if (!used[j] && x[j]==xi && y[j]==yi && z[j]==zi) {
                if(abs(mag_x[j]-mag_x[j-1])/mag_x[j-1] > 0.1 ||
                   abs(mag_y[j]-mag_y[j-1])/mag_y[j-1] > 0.1 ||
                   abs(mag_z[j]-mag_z[j-1])/mag_z[j-1] > 0.1) {
                    continue; // 跳過異常值
                }
                sum_magx += mag_x[j];
                sum_magy += mag_y[j];
                sum_magz += mag_z[j];
                sum_magnitude += magnitude[j];
                sum_heading += heading[j];
                used[j] = true;
                count++;
            }
        }

        if (count > 0) {
            fprintf(out, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%d\n",
                    xi, yi, zi,
                    sum_magx/count,
                    sum_magy/count,
                    sum_magz/count,
                    sum_magnitude/count,
                    sum_heading/count,
                    count);
        }
    }

    fclose(out);
    printf("result.csv finished\n");

    free(x); free(y); free(z);
    free(num); free(mag_x); free(mag_y); free(mag_z);
    free(magnitude); free(heading); free(used);

    return 0;
}
