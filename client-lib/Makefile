CC = gcc
CFLAGS = -Wall -Wextra -g -fPIC

# Library (chỉ build library cho Qt sử dụng)
LIB_NAME = libbattleship.a
LIB_SOURCES = battleship_client.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

all: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJECTS)
	ar rcs $@ $^
	@echo "Built library: $(LIB_NAME)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJECTS) $(LIB_NAME)

.PHONY: all clean
