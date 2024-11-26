#ifndef FT_PING_H
#define FT_PING_H

#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>

enum options {
    HELP = 1,
    VERBOSE = 2,
    DOMAIN = 3,
  };

typedef struct arguments {
    char *hostname;
    char *ip;
    enum options option;
    char *invalid_arg;
} t_args;

void  check_args(char **argv, t_args *args);
t_args *get_new_args();

#endif