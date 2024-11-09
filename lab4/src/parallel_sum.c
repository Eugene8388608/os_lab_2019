#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <sys/time.h>

#include "find_sum.h"
#include "../../lab3/src/utils.h"

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

#define SET_POSITIVE_INT_ARG(var, i) {\
  var = atoi(argv[i]);\
  if (var <= 0) {\
    printf("%s is a positive number\n", #var);\
    return 1;\
  }\
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  if (argc != 4) {
    printf("Usage: %s threads_num arraysize seed\n", argv[0]);
    return 1;
  }

  SET_POSITIVE_INT_ARG(threads_num, 1);
  SET_POSITIVE_INT_ARG(array_size, 2);
  SET_POSITIVE_INT_ARG(seed, 3);

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  pthread_t *threads = calloc(threads_num, sizeof(pthread_t));
  struct SumArgs *args = calloc(threads_num, sizeof (struct SumArgs));

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i] = (struct SumArgs){
      array,
      (size_t)array_size * i / threads_num,
      (size_t)array_size * (i+1) / threads_num
    };

    if (pthread_create(&threads[i], NULL, ThreadSum, args + i)) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}
