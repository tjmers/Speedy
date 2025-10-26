# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23 -I"C:\Users\jacob_\Downloads\boost_1_89_0\boost_1_89_0"
LDFLAGS := -ld2d1 -ldwrite -lws2_32

# Directories
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := tests
TEST_BUILD_DIR := $(BUILD_DIR)/test

# Files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
TARGET := Speedy.exe

# Test files
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp, $(TEST_BUILD_DIR)/%.o, $(TEST_SRCS))
TEST_TARGET := tests.exe

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Force Windows-friendly directory separators
TEST_BUILD_DIR_WIN := $(subst /,\,$(TEST_BUILD_DIR))
BUILD_DIR_WIN := $(subst /,\,$(BUILD_DIR))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	if not exist "$(BUILD_DIR_WIN)" mkdir "$(BUILD_DIR_WIN)"
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ===== Test build =====
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CXX) $(OBJS) $(TEST_OBJS) -o $@ $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	if not exist "$(TEST_BUILD_DIR_WIN)" mkdir "$(TEST_BUILD_DIR_WIN)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	del /Q $(BUILD_DIR)\*.o $(TEST_BUILD_DIR)\*.o $(TARGET) $(TEST_TARGET) 2>nul || exit 0
	del config/speedy.cfg config/commands.cfg

.PHONY: all clean test
