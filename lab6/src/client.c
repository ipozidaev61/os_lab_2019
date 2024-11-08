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
#include "multimodulo.h"

struct Server {
  char ip[255];
  int port;
};

struct ThreadData {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
    uint64_t result;
};


bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void *ThreadFunc(void *args) {
    struct ThreadData *data = (struct ThreadData *)args;
    
    struct hostent *hostname = gethostbyname(data->server.ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", data->server.ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(data->server.port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    uint64_t begin = data->begin;
    uint64_t end = data->end;
    uint64_t mod = data->mod;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }

    memcpy(&data->result, response, sizeof(uint64_t));

    close(sck);
    return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255
  //255 is max len of a file name

  while (true) {
    int current_optind = optind ? optind : 1;

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
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        if (k <= 0) 
        {
              printf("k should be > 0\n");
              return 1;
        }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        if (mod <= 0) 
        {
              printf("mod should be > 0\n");
              return 1;
        }
        break;
      case 2: ;
        // TODO: your code here
        FILE *file;
        if ((file = fopen(optarg, "r")) == NULL)
        {
            printf("file not found\n");
            return 1;
        }
        fclose(file);
        memcpy(servers, optarg, strlen(optarg));
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

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  unsigned int servers_num = 0;
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(servers, "r");
  if (fp == NULL)
      exit(EXIT_FAILURE);

  char c;    
  for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') 
            servers_num += 1;

  rewind(fp);

  struct Server *to = malloc(sizeof(struct Server) * servers_num);

  for (int i = 0; i < servers_num; i++) {
      read = getline(&line, &len, fp);
      sscanf(line, "%255[^:]:%d", to[i].ip, &to[i].port);
  }

  fclose(fp);
  if (line)
      free(line);

  pthread_t threads[servers_num];
  struct ThreadData thread_data[servers_num];
  uint64_t range_per_server = k / servers_num;
    
  for (int i = 0; i < servers_num; i++) {
      thread_data[i].server = to[i];
      thread_data[i].begin = i * range_per_server + 1;
      thread_data[i].end = (i == servers_num - 1) ? k : (i + 1) * range_per_server;
      thread_data[i].mod = mod;
      pthread_create(&threads[i], NULL, ThreadFunc, &thread_data[i]);
  }
    
  uint64_t final_result = 1;
  for (int i = 0; i < servers_num; i++) {
      pthread_join(threads[i], NULL);
      final_result = MultModulo(final_result, thread_data[i].result, mod);
  }
  printf("Final result: %llu\n", final_result);

  free(to);

  return 0;
}
