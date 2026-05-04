NAME		=	ircserv

C++			=	c++

CFLAGS		=	-Wall -Werror -Wextra -I./includes -std=c++98

SRCS_DIR	=	src/
OBJS_DIR	=	obj/

UTILS_DIR	=	utils/
UTILS		=	

ENTRY		=	main

SRC_FILES	+=	$(addprefix $(UTILS_DIR), $(UTILS))
SRC_FILES	+=  $(ENTRY)

SRCS 		= 	$(addprefix $(SRCS_DIR), $(addsuffix .c, $(SRC_FILES)))
OBJS 		= 	$(addprefix $(OBJS_DIR), $(addsuffix .o, $(SRC_FILES)))

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
