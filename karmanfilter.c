#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define P_Q 0.1  //process noise covariance
#define M_R 0.01 //measurement noise covariance
// 函式宣告
int getmaxnum(const char *filename);
float calcul_average(float* input, int MAXNUM);
float error2(float* mag, float average, int MAXNUM);
void noise(float* data, float* noise, float average, int MAXNUM);

int main() {
    const char *filename = "Flight_itrik300_.csv";
    int MAXNUM = getmaxnum(filename); // 先取得資料筆數
    // 配置記憶體
    int* num = (int*)malloc(MAXNUM * sizeof(int));
    float* mag_x = (float*)malloc(MAXNUM * sizeof(float));
    float* mag_y = (float*)malloc(MAXNUM * sizeof(float));
    float* mag_z = (float*)malloc(MAXNUM * sizeof(float));   
    float* magnitude = (float*)malloc(MAXNUM * sizeof(float));
    float* heading = (float*)malloc(MAXNUM * sizeof(float));
    float* mag_x_error = (float*)malloc(MAXNUM * sizeof(float));
    float* mag_y_error = (float*)malloc(MAXNUM * sizeof(float));
    float* mag_z_error = (float*)malloc(MAXNUM * sizeof(float));
    char (*time)[16] = malloc(MAXNUM * 16 * sizeof(char));

    FILE *fp = fopen(filename, "r"); 
    if (fp == NULL) {
        fprintf(stderr, "fopen() failed.\n");
        exit(EXIT_FAILURE);
    }

    char line[200];
    int i = 0;

    while (fgets(line, sizeof(line), fp) != NULL && i < MAXNUM) {
        sscanf(line, "%15[^,],%d,%f,%f,%f,%f,%f",
            time[i], &num[i], &mag_x[i], &mag_y[i], &mag_z[i],
            &magnitude[i], &heading[i]);

        printf("%d %.0f %.0f %.0f %.2f %.2f\n", num[i], mag_x[i], mag_y[i], mag_z[i], magnitude[i], heading[i]);
        i++;
    }
    fclose(fp); // 關檔案
    File*result=fopen("result.csv", "w+");
    //calculate average
    float mag_x_ave=calculate_average(mag_x, 120);
    float mag_y_ave=calculate_average(mag_y, 120);
    float mag_z_ave=calculate_average(mag_z,120);
    float mag_magnitude_ave=calculate_average(magnitude, 120);
    float mag_heading_ave=calculate_average(heading, 120);
    printf("Average X: %.2f  ", mag_x_ave);
    printf("Average Y: %.2f  ", mag_y_ave);
    printf("Average Z: %.2f  ", mag_z_ave);
    printf("Average Magnitude: %.2f ", mag_magnitude_ave);
    printf("Average Heading: %.2f ", mag_heading_ave);
    cal_sqdiff(mag_x, mag_x_error, mag_x_ave, MAXNUM);
    cal_sqdiff(mag_y, mag_y_error, mag_y_ave, MAXNUM);
    cal_sqdiff(mag_z, mag_z_error, mag_z_ave, MAXNUM);
    
    
    free(num);
    free(mag_x);
    free(mag_y);
    free(mag_z);
    free(magnitude);
    free(heading);
    free(time);

    printf("Data processing completed successfully.\n");
    return 0;
}
int getmaxnum(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "fopen() failed in getmaxnum().\n");
        return -1;
    }
    char line[200];
    int last_num = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        int num;
        if (sscanf(line, "%*[^,],%d", &num) == 1) {
            last_num = num;
        }
    }
    fclose(fp);
    return last_num;
}
float calculate_average(float* input,int MAXNUM) {
    float sum = 0.0;
    int count = 0;
    for (int i = 0; i < MAXNUM; i++) {
        if (input[i] != 0) {
            sum += input[i];
            count++;
        }
    }
    return (count > 0) ? (sum / count) : 0.0;
}

float error2(float* mag, float average, int MAXNUM) {
    float sum = 0.0;
    int count = 0;
    for (int i = 0; i < MAXNUM; i++) {
        if (mag[i] != 0) {
            sum += (mag[i] - average) * (mag[i] - average);
            count++;
        }
    }
    return (count > 0) ? sqrt(sum / count) : 0.0;
}
void cal_sqdiff(float* data,float* noise,float average, int MAXNUM) {
    for (int i = 0; i < MAXNUM; i++) {
            noise[i]= (data[i] - average) * (data[i] - average);
    }
    return 0;
}
float p_count(){}
