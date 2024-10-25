#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <stdbool.h>
#include <pthread.h>

#include "task1/utils.h"
#include "sum.h"

struct SumArgs {
  int *array;
  int begin;
  int end;
};

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    long long *result = malloc(sizeof(long long));
    *result = Sum(sum_args);
    return result;
}

int main(int argc, char **argv) {

  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"array_size", required_argument, 0, 0},
                                      {"threads_num", required_argument, 0, 0},
                                      {"seed", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size should be > 0\n");
              return 1;
            }
            break;
          case 1:
            threads_num = atoi(optarg);
            if (threads_num <= 0 || threads_num > array_size) {
              printf("threads_num should be > 0 and <= array_size\n");
              return 1;
            }
            break;
          case 2:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed should be > 0\n");
              return 1;
            }
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (threads_num == 0 || array_size == 0 || seed == 0) {
    printf("Usage: %s --threads_num \"num\" --array_size \"num\" --seed \"num\"\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  pthread_t threads[threads_num];
  struct SumArgs args[threads_num];
  int segment_size = array_size / threads_num;

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * segment_size;
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * segment_size;
  }

  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (uint32_t i = 0; i < threads_num; i++) {
        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            free(array);
            return 1;
        }
    }

  long long total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
        long long *sum;
        pthread_join(threads[i], (void **)&sum);
        total_sum += *sum; 
        free(sum);
  }

  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

  free(array);
  printf("Total: %lld\n", total_sum);
  printf("Elapsed time: %.6f seconds\n", elapsed_time);
  return 0;
}
