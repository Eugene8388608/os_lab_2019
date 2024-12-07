#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"

#include "utils.h"

#define SET_POSITIVE_INT_OPTARG(var) {\
  var = atoi(optarg);\
  if (var <= 0) {\
    printf("%s is a positive number\n", #var);\
    return 1;\
  }\
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t res = 1;

  for (__uint128_t i = fargs->begin; i <= fargs->end; i++)
    res = res * i % fargs->mod;

  // for (uint64_t i = fargs->begin; i, i <= fargs->end; i++)
  //   res = MultModulo(res, i, fargs->mod);

  return (void *)res;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        SET_POSITIVE_INT_OPTARG(port);
        break;
      case 1:
        SET_POSITIVE_INT_OPTARG(tnum);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Can not create server socket");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    perror("Can not bind to socket");
    return 1;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    perror("Could not listen on socket");
    return 1;
  }

  printf("Server listening at %d\n", port);

  pthread_t *threads = calloc(tnum, sizeof(pthread_t));
  struct FactorialArgs *args = calloc(tnum, sizeof(struct FactorialArgs));

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      perror("Could not establish new connection");
      continue;
    }

    while (true) {
      struct FactorialArgs task;
      int read = recv(client_fd, &task, sizeof task, 0);

      if (!read)
        break;
      if (read < 0) {
        perror("Client read failed");
        break;
      }
      if (read < sizeof task) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      printf("Receive: %lu %lu %lu\n", task.begin, task.end, task.mod);

      task.begin--;
      uint64_t delta = task.end - task.begin;
      for (__uint128_t i = 0; i < tnum; i++) {
        args[i].begin = task.begin + delta * i / tnum + 1;
        args[i].end = task.begin + delta * (i + 1) / tnum;
        args[i].mod = task.mod;

        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, task.mod);
      }

      printf("Total: %lu\n", total);

      err = send(client_fd, &total, sizeof(total), 0);
      if (err < 0) {
        perror("Can't send data to client");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}
