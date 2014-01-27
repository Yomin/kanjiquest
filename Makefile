
.PHONY: all, debug, check

NAME = kanjiquest
CFLAGS = -Wall

all: DEBUG =
all: CFLAGS := $(CFLAGS)
all: check $(NAME)

debug: DEBUG = debug
debug: CFLAGS := $(CFLAGS) -ggdb
debug: check $(NAME)

$(NAME): $(NAME).c
	gcc $(CFLAGS) $< -o $@ $$(pkg-config --cflags --libs gtk+-3.0)

check:
	$(shell [ -f $(NAME) -a -z "$(DEBUG)" ] &&   objdump --debugging $(NAME)|grep -q gdb && rm $(NAME))
	$(shell [ -f $(NAME) -a -n "$(DEBUG)" ] && ! objdump --debugging $(NAME)|grep -q gdb && rm $(NAME))
