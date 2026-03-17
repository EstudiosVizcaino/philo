NAME		= philo_bonus

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
RM			= rm -f

SRCS		= main_bonus.c init_bonus.c routine_bonus.c \
			monitor_bonus.c utils_bonus.c
OBJS		= $(SRCS:.c=.o)
HEADER		= philo_bonus.h

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o:		%.c $(HEADER)
			$(CC) $(CFLAGS) -c $< -o $@

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
