SRC = main.c \
			ft_ping.c \
			ping_parser.c \
			send_ping.c \
			receive_ping.c


BNS = main.c

CFLAGS = -Wall -Wextra -Werror

NAME = ft_ping

NAME_BNS = ft_ping_bonus

OBJECT = ${SRC:.c=.o}

BNS_OBJ = ${BNS:.c=.o}

all : $(NAME)
$(NAME): $(OBJECT)
	@$(CC) $(CFLAGS) $(OBJECT) -o $(NAME)
	@printf "\033[0;32mft_ping\033[0m\n"

bonus : $(NAME_BNS)
$(NAME_BNS): $(BNS_OBJ)
	@$(CC) $(CFLAGS) $(BNS_OBJ) -o $(NAME_BNS)
	@printf "\033[0;32mft_ping\033[0m\n"


clean :
	@rm -f $(OBJECT) $(BNS_OBJ) $(LIB)
	@printf "\033[0;31mDeleted\033[0m\n"

fclean : clean
	@rm -f $(NAME)
	@rm -f $(NAME_BNS)

re : fclean all

.PHONY : all re bonus flcean clean