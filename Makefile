CC := gcc
CFLAGS := -lX11 -lXi #-Wall -Wextra -Werror

SRC_DIRS := src
INCLUDE_DIR := include
BUILD_DIR := build

SRC := touchpad_zoom.c init.c zoom.c
INCLUDE := touchpad_zoom.h 

INCLUDE := $(addprefix $(INCLUDE_DIR)/,$(INCLUDE))

OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)

NAME := tz

all: $(NAME)

$(NAME): $(OBJ) $(INCLUDE)
	@$(CC) $(OBJ) $(CFLAGS) -o $(NAME) 

$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.c $(INCLUDE)
	@[ -d "$(BUILD_DIR)" ] || mkdir "$(BUILD_DIR)"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -rf $(NAME) $(BUILD_DIR)

re: fclean all

.PHONY: all clean fclean re 
