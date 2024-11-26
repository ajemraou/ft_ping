#include "ft_ping.h"

int main(int ac, char *av[]) {
  t_args *args;
  if (ac < 2){
    return (0);
  }
  args = get_new_args();
  check_args(av, args);
  printf("=============================\n");
  printf("hostname: %s\n", args->hostname);
  printf("ip: %s\n", args->ip);
  printf("option: %d\n", args->option);
  printf("invalid_arg: %s\n", args->invalid_arg);
  printf("=============================\n");
  return 0;
}