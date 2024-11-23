#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <sys/time.h>

struct FactArgs {
  uint64_t begin, end, mod;
};

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
uint64_t res = 1;

void *ThreadFact(void *args) {
  struct FactArgs *fact_args = (struct FactArgs *)args;

  for (__uint128_t i = fact_args->begin; i <= fact_args->end; i++) {
    pthread_mutex_lock(&mut);
    uint64_t temp = res = res * i % fact_args->mod;
    pthread_mutex_unlock(&mut);

    if (temp == 0) break;
  }

  return NULL;
}

#define SET_UINT64_ARG(var, i, min) {\
  char *end;\
  var = strtoull(argv[i], &end, 10);\
  if (errno || argv[i] + strlen(argv[i]) != end || var < (min)) {\
    printf("%s is a 64-bit number not less than %d\n", #var, (min));\
    return 1;\
  }\
}

int main(int argc, char **argv) {
  uint64_t threads_num, k, mod;

  if (argc != 4) {
    printf("Usage: %s threads_num k mod\n", argv[0]);
    return 1;
  }

  SET_UINT64_ARG(threads_num, 1, 1);
  SET_UINT64_ARG(k, 2, 2);
  SET_UINT64_ARG(mod, 3, 2);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  pthread_t *threads = calloc(threads_num, sizeof(pthread_t));
  struct FactArgs *args = calloc(threads_num, sizeof (struct FactArgs));

  for (__uint128_t i = 0; i < threads_num; i++) {
    args[i] = (struct FactArgs){
      k * i / threads_num + 1,
      k * (i+1) / threads_num,
      mod
    };

    if (pthread_create(threads + i, NULL, ThreadFact, args + i)) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  for (uint64_t i = 0; i < threads_num; i++)
    pthread_join(threads[i], NULL);

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(threads);
  free(args);
  printf("Result: %ld\n", res);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}
