# Makefile for BattleShip Server

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
TARGET = server_full
SOURCE = server_full.cpp

# Directories
HISTORY_DIR = history

# Default target
all: $(TARGET)

# Build server
$(TARGET): $(SOURCE)
	@echo "Compiling $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "Build successful! Executable: $(TARGET)"

# Run server
run: $(TARGET)
	@mkdir -p $(HISTORY_DIR)
	@echo "Starting server..."
	./$(TARGET)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET)
	@echo "Clean complete!"

# Clean everything including data files
cleanall: clean
	@echo "Cleaning all data files..."
	rm -f users.dat
	rm -rf $(HISTORY_DIR)
	@echo "All data cleaned!"

# Rebuild (clean + build)
rebuild: clean all

# Create necessary directories
setup:
	@mkdir -p $(HISTORY_DIR)
	@echo "Setup complete!"

# Show help
help:
	@echo "BattleShip Server Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make           - Build the server (default)"
	@echo "  make run       - Build and run the server"
	@echo "  make clean     - Remove executable"
	@echo "  make cleanall  - Remove executable and all data files"
	@echo "  make rebuild   - Clean and rebuild"
	@echo "  make setup     - Create necessary directories"
	@echo "  make help      - Show this help message"

.PHONY: all run clean cleanall rebuild setup help
