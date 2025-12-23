CC = gcc
CFLAGS = -Wall -pthread
TARGET = client
SRC = client.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
	@echo "C client compiled successfully"

clean:
	rm -f $(TARGET)
	@echo "Cleaned up"

.PHONY: all clean
