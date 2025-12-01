# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I/usr/include/mysql

# Linker flags
LDFLAGS = -lmysqlclient

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Database.cpp \
          $(SRC_DIR)/UIHelper.cpp \
          $(SRC_DIR)/Config.cpp \
          $(SRC_DIR)/BaseController.cpp \
          $(SRC_DIR)/AdminController.cpp \
          $(SRC_DIR)/TeacherController.cpp \
          $(SRC_DIR)/StudentController.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = attendance_system

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
