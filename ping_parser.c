#include "ft_ping.h"

int resolve_hostname(const char *hostname, struct sockaddr_in *dest_addr) {
  struct addrinfo hints, *result;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;

  if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
      return -1;
  }
  memcpy(&dest_addr->sin_addr, &((struct sockaddr_in *)result->ai_addr)->sin_addr, sizeof(dest_addr->sin_addr));
  freeaddrinfo(result);
  return 0;
}

bool is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 0 ? false : true;
}

bool is_valid_domain(const char *domain, struct sockaddr_in *dest_addr) {
  if (is_valid_ip(domain) == true) {
      return true;
  }
  if (resolve_hostname(domain, dest_addr) == -1) {
    return  false;
  };
  return is_valid_ip(inet_ntoa(dest_addr->sin_addr));
}

void fill_args(t_args *arg, char *hostname) {
  struct sockaddr_in  dest_addr;
  if (is_valid_domain(hostname, &dest_addr) == false){
      free(arg->invalid_arg);
    arg->invalid_arg = strdup(hostname);
    return ;
  }
  free(arg->hostname);
  free(arg->ip);
  arg->hostname = strdup(hostname);
  if (is_valid_ip(hostname) == true){
    arg->ip = strdup(hostname);
  }
  else {
    arg->ip = strdup(inet_ntoa(dest_addr.sin_addr));
  }
}

enum options is_option(char *arg) {
  if (strcmp(arg, "-?") == 0 || strcmp(arg, "--help") == 0) {
    return HELP;
  } else if (strcmp(arg, "-v") == 0) {
    return VERBOSE;
  }
  return DOMAIN;
}

void  check_args(char **argv, t_args *args) {
  enum options  option;
  int           i;

  i = 1;
  while (argv[i]) {
    option = is_option(argv[i]);
    if (option == HELP) {
      args->option = HELP;
      break;
    }
    else if (option == VERBOSE) {
      args->option = VERBOSE;
    }
    else if (option == DOMAIN) {
      fill_args(args, argv[i]);
    }
    i++;
  }
}

t_args *get_new_args() {
  t_args *args = malloc(sizeof(t_args));
  args->hostname = NULL;
  args->ip = NULL;
  args->option = 0;
  args->invalid_arg = NULL;
  args->packets_sent = 0;
  return args;
}
