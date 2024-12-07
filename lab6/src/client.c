#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"

#define SET_UINT64_OPTARG(var, min) {\
  char *end;\
  var = strtoull(optarg, &end, 10);\
  if (errno || optarg + strlen(optarg) != end || var < (min)) {\
    printf("%s is a 64-bit number not less than %d\n", #var, (min));\
    return 1;\
  }\
}

char fscan_address_ip[256];
uint16_t fscan_address_port;
FILE* fscan_address_file;

int fscan_address() {
  return fscanf(
    fscan_address_file,
    "%255[0123456789.]:%hu\n",
    fscan_address_ip,
    &fscan_address_port
  );
}

uint64_t k = -1;
uint64_t mod = -1;
unsigned int servers_num = 0;

uint64_t recursive_request(int i);

int main(int argc, char **argv) {
  char* servers = NULL;

  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        SET_UINT64_OPTARG(k, 1);
        break;
      case 1:
        SET_UINT64_OPTARG(mod, 2);
        break;
      case 2:
        servers = optarg;
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || servers == NULL) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  fscan_address_file = fopen(servers, "r");
  if (fscan_address_file == NULL) {
    perror("Error opening file");
    return 1;
  }

  for (int r = fscan_address(); r != EOF; r = fscan_address()) {
    if (r != 2) {
      fprintf(stderr, "Wrong file format\n");
      return 1;
    }

    servers_num++;
  }

  if (servers_num == 0) {
    fprintf(stderr, "Empty file\n");
    return 1;
  }

  rewind(fscan_address_file);

  uint64_t answer = recursive_request(0);

  printf("answer: %lu\n", answer);

  return 0;
}

uint64_t recursive_request(int i) {
  if (i == servers_num) return 1;

  fscan_address();
  struct hostent *hostname = gethostbyname(fscan_address_ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", fscan_address_ip);
    exit(1);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(fscan_address_port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    perror("Socket creation failed");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("Connection failed");
    exit(1);
  }

  struct FactorialArgs task = {
    begin: k * (__uint128_t)i / servers_num + 1,
    end: k * (__uint128_t)(i + 1) / servers_num,
    mod: mod
  };

  if (send(sck, &task, sizeof task, 0) < 0) {
    perror("Send failed");
    exit(1);
  }

  //ни один recv ещё не вызван
  uint64_t answer = recursive_request(i + 1);
  //теперь, может быть, вызван

  uint64_t response;
  if (recv(sck, &response, sizeof(response), 0) < 0) {
    perror("Receive failed");
    exit(1);
  }

  close(sck);

  return MultModulo(answer, response, mod);
}
