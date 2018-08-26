NAME = LZW
CC = gcc
CFLAGS = -Wall -Wextra
FILE = LZW.c
OBJ = $(FILE:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ)

$(OBJ): $(FILE)
	$(CC) -c $(FILE) $(CFLAGS)

clean:
	rm $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
