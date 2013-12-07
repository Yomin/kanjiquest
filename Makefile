
NAME=kanjiquest

all: $(NAME)

$(NAME): $(NAME).c
	gcc -Wall $< -o $@ $$(pkg-config --cflags --libs gtk+-3.0)
