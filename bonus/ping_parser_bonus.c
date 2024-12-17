#include "ft_ping_bonus.h"

int parse_int(const char *value, int min, int max, const char *flag) {
    char  *endptr;
    long  result;

    errno = 0;
    result = strtol(value, &endptr, 10);
    if (errno == ERANGE || result > max || result < min) {
        fprintf(stderr, "Error: %s value '%s' is out of range (%d to %d).\n", flag, value, min, max);
        exit(EXIT_FAILURE);
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid %s value '%s'.\n", flag, value);
        exit(EXIT_FAILURE);
    }

    return (int)result;
}

float parse_float(const char *value, float min, float max, const char *flag) {
    char    *endptr;
    double  result;

    result = strtod(value, &endptr);
    errno = 0;
    if (errno == ERANGE || result > max || result < min) {
        fprintf(stderr, "Error: %s value '%s' is out of range (%.1f to %.1f).\n", flag, value, min, max);
        exit(EXIT_FAILURE);
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid %s value '%s'.\n", flag, value);
        exit(EXIT_FAILURE);
    }

    return (float)result;
}

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

PingFlags  get_option( const char *arg ){
  if (strcmp(arg, "-c") == 0){
    return FLAG_COUNT;
  }
  else if (strcmp(arg, "-i") == 0){
    return FLAG_INTERVAL;
  }
  else if (strcmp(arg, "-w") == 0){
    return FLAG_TIMEOUT;
  }
  else if (strcmp(arg, "-W") == 0){
    return FLAG_REPLY_TIMEOUT;
  }
  else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--ttl") == 0){
    return FLAG_TTL;
  }
  else if (strcmp(arg, "--usage") == 0){
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
  while (i < argc)
  {
      if (get_option(argv[i]) == FLAG_HELP) {
        options->option = FLAG_HELP;
        break;
      } else if (get_option(argv[i]) == FLAG_VERBOSE){
        options->option = FLAG_VERBOSE;
      } else if (get_option(argv[i]) == FLAG_USAGE) {
        options->option = FLAG_USAGE;
        break;
      } else if (get_option(argv[i]) == FLAG_INTERVAL && i + 1 < argc) {
          options->interval = parse_int(argv[++i], 1, MAX_INTERVAL, "-i");
      } else if (get_option(argv[i]) == FLAG_TTL && i + 1 < argc) {
          options->ttl = parse_int(argv[++i], 1, MAX_TTL, "--ttl");
      } else if (get_option(argv[i]) == FLAG_TIMEOUT && i + 1 < argc) {
          options->timeout = parse_int(argv[++i], 1, INT_MAX, "-w");
      } else if (get_option(argv[i]) == FLAG_REPLY_TIMEOUT && i + 1 < argc) {
          options->reply_timeout = parse_float(argv[++i], MIN_REPLY_TIMEOUT, FLT_MAX, "-W");
      } else if (get_option(argv[i]) == FLAG_COUNT && i + 1 < argc) {
          options->count = parse_int(argv[++i], 1, INT_MAX, "-c");
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
  args->option = -1;
  args->invalid_arg = NULL;
  args->packets_sent = 0;
  args->interval = 1;
  args->ttl = 64;
  args->timeout = 0;
  args->reply_timeout = RECV_TIMEOUT;
  args->count = -1;
  return args;
}
