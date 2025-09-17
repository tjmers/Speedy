# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23
LDFLAGS := -ld2d1 -ldwrite

# Directories
SRC_DIR := src
BUILD_DIR := build

# Files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
TARGET := Speedy.exe

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile step (Windows-friendly mkdir)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	del /Q $(BUILD_DIR)\*.o $(TARGET) 2>nul || exit 0
	del config/speedy.cfg config/commands.cfg

.PHONY: all clean
