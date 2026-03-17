NAME		= philo

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
RM			= rm -f

SRCS		= main.c init.c routine.c monitor.c utils.c
OBJS		= $(SRCS:.c=.o)
HEADER		= philo.h

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

docs:
			doxygen Doxyfile

.PHONY:		all clean fclean re docs