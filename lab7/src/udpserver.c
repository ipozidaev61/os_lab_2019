#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  int sockfd, n;
  int port, bufsize;
  char *mesg, ipadr[16];
  struct sockaddr_in servaddr, cliaddr;

  if (argc != 3) {
    printf("Usage: %s <Port> <Buffer Size>\n", argv[0]);
    exit(1);
  }

  port = atoi(argv[1]);
  bufsize = atoi(argv[2]);

  mesg = malloc(bufsize);
  if (!mesg) {
    perror("Buffer allocation failed");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  memset(&servaddr, 0, SLEN);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0) {
    perror("bind problem");
    exit(1);
  }

  printf("SERVER starts...\n");

  while (1) {
    unsigned int len = SLEN;

    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0) {
      perror("recvfrom problem");
      exit(1);
    }
    mesg[n] = 0;  // Null-terminate the received string

    printf("REQUEST %s FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipadr, sizeof(ipadr)),
           ntohs(cliaddr.sin_port));

    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
      perror("sendto problem");
      exit(1);
    }
  }

  free(mesg);
  close(sockfd);
  exit(0);
}
