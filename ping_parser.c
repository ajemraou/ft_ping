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

void fill_args(t_args *arg, const char *hostname) {
  struct sockaddr_in  dest_addr;
  if (is_valid_domain(hostname, &dest_addr) == false){
      free(arg->invalid_arg);
    arg->invalid_arg = strdup(hostname);
    return ;
  }
  if (!arg->hostname){
    arg->hostname = strdup(hostname);
    if (is_valid_ip(hostname) == true){
      arg->ip = strdup(hostname);
    }
    else {
      arg->ip = strdup(inet_ntoa(dest_addr.sin_addr));
    }
  }
}

PingFlags  get_option( const char *arg ){
  if (strcmp(arg, "--usage") == 0){
    return FLAG_USAGE;
  }
  else if (strcmp(arg, "-?") == 0 || strcmp(arg, "--help") == 0) {
    return FLAG_HELP;
  }
  else if (strcmp(arg, "-v") == 0) {
    return FLAG_VERBOSE;
  }
  return FLAG_DOMAIN;
}

void parse_flags(int argc, char *argv[], t_args *options) {
  int i;

  i = 1;
  while (i < argc && !options->invalid_arg)
  {
      if (get_option(argv[i]) == FLAG_HELP) {
        options->option = FLAG_HELP;
        break;
      } else if (get_option(argv[i]) == FLAG_VERBOSE){
        options->option = FLAG_VERBOSE;
      } else if (get_option(argv[i]) == FLAG_USAGE) {
        options->option = FLAG_USAGE;
        break;
      } else if (get_option(argv[i]) == FLAG_DOMAIN) {
        fill_args(options, argv[i]);
      } else {
          fprintf(stderr, "Error: Unknown or incomplete flag '%s'.\n", argv[i]);
          exit(EXIT_FAILURE);
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
  args->identifier = 0;
  return args;
}
