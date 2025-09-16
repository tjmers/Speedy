# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23
LDFLAGS := -ld2d1 -ldwrite

# Files
SRCS := speedy.cpp graphics.cpp client.cpp opened_file.cpp config.cpp edit.cpp command.cpp command_controller.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := Speedy.exe

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile step
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	del /Q $(OBJS) $(TARGET) 2>nul || exit 0
	del speedy.cfg commands.cfg

.PHONY: all clean
