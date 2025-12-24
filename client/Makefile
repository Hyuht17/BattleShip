# Makefile for C Client Bridge

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = client
SRC = client.c

# Default target
all: $(TARGET)

# Build the client
$(TARGET): $(SRC)
	@echo "Compiling $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
	@echo "Build successful! Executable: $(TARGET)"

# Run the client (default: port 9000, localhost, port 8080)
run: $(TARGET)
	@echo "Running client on port 9000, connecting to localhost:8080..."
	./$(TARGET) 9000 localhost 8080

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET)
	@echo "Clean complete!"

# Clean and rebuild
rebuild: clean all

# Setup (create any necessary directories)
setup:
	@echo "No additional setup needed for client"

# Help
help:
	@echo "Available targets:"
	@echo "  make          - Build the client"
	@echo "  make run      - Build and run client (default: 9000 localhost 8080)"
	@echo "  make clean    - Remove compiled files"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  ./client 9000 192.168.1.100 8080"
	@echo "  ./client 9000 example.com 8080"
	@echo "  ./client 9000 localhost"

.PHONY: all run clean rebuild setup help
