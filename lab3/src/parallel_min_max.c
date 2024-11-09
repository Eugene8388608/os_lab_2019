#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>
#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

#define SET_POSITIVE_INT_OPTARG(var) {\
  var = atoi(optarg);\
  if (var <= 0) {\
    printf("%s is a positive number\n", #var);\
    return 1;\
  }\
}

int get_file_name(char *fname, size_t fname_size, int i, struct timeval start_time) {
  return snprintf(fname, fname_size, ".parallel_min_max_%d_%jd", i, start_time.tv_sec);
}

int pnum = -1;
int *child_pids;

void alarm_handler(int signum) {
  if (signum == SIGALRM)
    for (int i = 0; i < pnum; i++)
      kill(child_pids[i], SIGINT);
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int timeout = 0;
  bool with_files = false;

  while (true) {
    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            SET_POSITIVE_INT_OPTARG(seed);
            break;
          case 1:
            SET_POSITIVE_INT_OPTARG(array_size);
            break;
          case 2:
            SET_POSITIVE_INT_OPTARG(pnum);
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = -1;
            SET_POSITIVE_INT_OPTARG(timeout);
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  int* pipes = calloc(pnum, sizeof (int));
  child_pids = calloc(pnum, sizeof (int));

  for (int i = 0; i < pnum; i++) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
      perror("pipe");
      return EXIT_FAILURE;
    }

    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        close(pipefd[0]);

        // parallel somehow
        unsigned int begin = (size_t)array_size * i / pnum;
        unsigned int end = (size_t)array_size * (i+1) / pnum;
        struct MinMax min_max = GetMinMax(array, begin, end);

        if (with_files) {
          //TODO: error handling
          
          char fname[64];
          get_file_name(fname, sizeof fname, i, start_time);
          
          FILE* f = fopen(fname, "wb");
          if (f == NULL) {
            perror("Error opening temp file");
            return 1;
          }
          fwrite(&min_max, sizeof min_max, 1, f);
          fclose(f);
        } else {
          // use pipe here
          write(pipefd[1], &min_max, sizeof min_max);
          close(pipefd[1]);
        }
        return 0;
      } else {
        child_pids[i] = child_pid;
      }

      close(pipefd[1]);
      pipes[i] = pipefd[0];

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  alarm(timeout);
  signal(SIGALRM, alarm_handler);

  while (active_child_processes > 0) {
    // your code here
    int wstatus = 0;
    pid_t pid = wait(&wstatus);

    if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus)) {
      printf("child didn't exit normally, wstatus = 0x%08x\n", wstatus);
    }

    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax cmm;

    if (with_files) {
      // read from files

      //TODO: error handling
      char fname[64];
      get_file_name(fname, sizeof fname, i, start_time);
      
      FILE* f = fopen(fname, "rb");
      if (f == NULL) {
        perror("Error opening temp file");
        return 1;
      }
      fread(&cmm, sizeof cmm, 1, f);
      fclose(f);
      remove(fname);
    } else {
      // read from pipes
      read(pipes[i], &cmm, sizeof cmm);
      close(pipes[i]);
    }

    if (cmm.min < min_max.min) min_max.min = cmm.min;
    if (cmm.max > min_max.max) min_max.max = cmm.max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
