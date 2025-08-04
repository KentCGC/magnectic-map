#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 10000
#define MAX_GROUP 1000
#define MAX_PER_GROUP 100

// 儲存每一筆資料
int num[MAX_LINES];
float mag_x[MAX_LINES], mag_y[MAX_LINES], mag_z[MAX_LINES], magnitude[MAX_LINES];

// 每組最多資料筆數：100
typedef struct {
    float mag_x[MAX_PER_GROUP];
    float mag_y[MAX_PER_GROUP];
    float mag_z[MAX_PER_GROUP];
    float magnitude[MAX_PER_GROUP];
    int count;
} Group;

Group groups[MAX_GROUP];

// 簡單一維高斯濾波器（3點）
void gaussian_filter_1d(float* input, float* output, int len) {
    if (len < 3) {
        // 資料不足則直接複製
        for (int i = 0; i < len; i++) output[i] = input[i];
        return;
    }
    output[0] = input[0];
    for (int i = 1; i < len - 1; i++) {
        output[i] = 0.25 * input[i - 1] + 0.5 * input[i] + 0.25 * input[i + 1];
    }
    output[len - 1] = input[len - 1];
}

int main() {
    FILE* file = fopen("Flight_itrik300_.csv", "r");
    if (!file) {
        printf("cant open data.csv\n");
        return 1;
    }

    char line[256];
    int total = 0;

    // 讀取 CSV
    while (fgets(line, sizeof(line), file) && total < MAX_LINES) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        int n;
        float x, y, z, mag;
        if (sscanf(line, "%*[^,],%d,%f,%f,%f,%f,%*f", &n, &x, &y, &z, &mag) == 5) {
            num[total] = n;
            mag_x[total] = x;
            mag_y[total] = y;
            mag_z[total] = z;
            magnitude[total] = mag;

            // 放入對應群組
            if (n >= 0 && n < MAX_GROUP && groups[n].count < MAX_PER_GROUP) {
                int idx = groups[n].count;
                groups[n].mag_x[idx] = x;
                groups[n].mag_y[idx] = y;
                groups[n].mag_z[idx] = z;
                groups[n].magnitude[idx] = mag;
                groups[n].count++;
            }
            total++;
        }
    }
    fclose(file);

    // 開啟輸出檔
    FILE* out = fopen("gaussian_result.csv", "w");
    fprintf(out, "num,mag_x_avg,mag_y_avg,mag_z_avg,magnitude_avg\n");

    // 對每組進行濾波與平均
    for (int n = 0; n < MAX_GROUP; n++) {
        if (groups[n].count > 0) {
            int cnt = groups[n].count;

            float fx[MAX_PER_GROUP], fy[MAX_PER_GROUP], fz[MAX_PER_GROUP], fmag[MAX_PER_GROUP];

            gaussian_filter_1d(groups[n].mag_x, fx, cnt);
            gaussian_filter_1d(groups[n].mag_y, fy, cnt);
            gaussian_filter_1d(groups[n].mag_z, fz, cnt);
            gaussian_filter_1d(groups[n].magnitude, fmag, cnt);

            float sum_x = 0, sum_y = 0, sum_z = 0, sum_mag = 0;
            for (int i = 0; i < cnt; i++) {
                sum_x += fx[i];
                sum_y += fy[i];
                sum_z += fz[i];
                sum_mag += fmag[i];
            }

            fprintf(out, "%d,%.2f,%.2f,%.2f,%.2f\n",
                    n,
                    sum_x / cnt,
                    sum_y / cnt,
                    sum_z / cnt,
                    sum_mag / cnt);
        }
    }

    fclose(out);
    printf("輸出完成：gaussian_result.csv\n");
    return 0;
}

