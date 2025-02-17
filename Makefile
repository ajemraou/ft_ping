SRC = main.c \
			ft_ping.c \
			ping_parser.c \
			send_ping.c \
			receive_ping.c

CFLAGS = -Wall -Wextra -Werror

NAME = ft_ping

OBJECT = ${SRC:.c=.o}


all : $(NAME)
$(NAME): $(OBJECT)
	@$(CC) $(CFLAGS) $(OBJECT) -o $(NAME)
	@printf "\033[0;32mft_ping\033[0m\n"

clean :
	@rm -f $(OBJECT) $(LIB)
	@printf "\033[0;31mDeleted\033[0m\n"

fclean : clean
	@rm -f $(NAME)

re : fclean all

.PHONY : all re flcean clean