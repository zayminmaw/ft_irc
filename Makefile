NAME		=	ircserv

C++			=	c++

CFLAGS		=	-Wall -Werror -Wextra -I./includes -std=c++98

SRCS_DIR	=	src/
OBJS_DIR	=	obj/

SRC_FILES	=	main Server Client Channel CommandRouter Reply

SRCS		=	$(addprefix $(SRCS_DIR), $(addsuffix .cpp, $(SRC_FILES)))
OBJS		=	$(addprefix $(OBJS_DIR), $(addsuffix .o, $(SRC_FILES)))

${OBJS_DIR}%.o: ${SRCS_DIR}%.cpp
	@mkdir -p ${dir $@}
	${C++} ${CFLAGS} -c $< -o $@

all: ${NAME}

${NAME}: ${OBJS}
	@$(C++) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf ${OBJS_DIR}

fclean: clean
	rm -rf ${NAME}

re: fclean all

.PHONY: all clean fclean re
