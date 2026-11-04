#include <stdio.h>
#include <math.h>

// 相関係数計算
double correlation(double *x, double *y, int n) {
  double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0;
  double sum_x2 = 0.0, sum_y2 = 0.0;

  for(int i = 0; i < n; i++) {
    sum_x += x[i];
    sum_y += y[i];
    sum_xy += x[i] * y[i];
    sum_x2 += x[i] * x[i];
    sum_y2 += y[i] * y[i];
  }

  double numerator = n * sum_xy - sum_x * sum_y;
  double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));

  if(denominator == 0) {
    return 0; // Avoid division by zero
  }

  return numerator / denominator;
}

int main(int argc, char *argv[]) {
  FILE *file;
  double weight[1024], ind[1024];
  int i = 0;

  // Usage
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <data_file>\n", argv[0]);
    return 1;
  }

  // File open
  file = fopen(argv[1], "r");
  if(file == NULL) {
    fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
    return 1;
  }

  // 値読み取り
  while(fscanf(file, "%lf %lf", &weight[i], &ind[i]) == 2) {
    i++;
    if (i >= 1024) {
      fprintf(stderr, "Error: Too many data points, maximum is 1024\n");
      return 1;
    }
  }

  // 相関計算
  double corr = correlation(weight, ind, i);
  printf("%lf\n", corr);

  fclose(file);
  return 0;
}