# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I../include
LDFLAGS = -lmysqlclient -lpthread

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
PARENT_SRC = ../src

# Target executable
TARGET = $(BIN_DIR)/api_server

# Source files
SOURCES = $(SRC_DIR)/api_server.cpp \
          $(PARENT_SRC)/Database.cpp \
          $(PARENT_SRC)/Config.cpp

# Object files
OBJECTS = $(OBJ_DIR)/api_server.o \
          $(OBJ_DIR)/Database.o \
          $(OBJ_DIR)/Config.o

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile api_server.cpp
$(OBJ_DIR)/api_server.o: $(SRC_DIR)/api_server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Database.cpp from parent directory
$(OBJ_DIR)/Database.o: $(PARENT_SRC)/Database.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Config.cpp from parent directory
$(OBJ_DIR)/Config.o: $(PARENT_SRC)/Config.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Run the server
run: $(TARGET)
	@echo "Starting API server..."
	@cd .. && ./web/$(TARGET)

# Rebuild everything
rebuild: clean all

.PHONY: all clean run rebuild directories
