NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -Iinclude

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/parsing.c \
			  $(SRC_DIR)/parsing_numeric.c \
			  $(SRC_DIR)/init.c \
			  $(SRC_DIR)/cleanup.c \
			  $(SRC_DIR)/coder_routine.c \
			  $(SRC_DIR)/dongle.c \
			  $(SRC_DIR)/dongle_cooldown.c \
			  $(SRC_DIR)/monitor.c \
			  $(SRC_DIR)/stop.c \
			  $(SRC_DIR)/scheduler.c \
			  $(SRC_DIR)/scheduler_init.c \
			  $(SRC_DIR)/heap.c \
			  $(SRC_DIR)/time_utils.c \
			  $(SRC_DIR)/log.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/codexion.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
