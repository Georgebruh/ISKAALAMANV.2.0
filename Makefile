# Compiler
CXX = g++

# Compiler flags
# Consider adding -g for debugging symbols if needed, e.g., CXXFLAGS = -std=c++11 -Wall -g
CXXFLAGS = -std=c++11 -Wall

# Executable name
TARGET = iskaalaman_system

# Source files - iskaalaman.cpp is now main.cpp effectively
# We should use main.cpp if iskaalaman.cpp was renamed, or stick to iskaalaman.cpp if it was just repurposed.
# Based on previous step, iskaalaman.cpp was repurposed to be main.cpp's content.
SRCS = iskaalaman.cpp utils.cpp file_handler.cpp scheduler_planner.cpp study_hub.cpp

# Object files: one .o for each .cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Linking rule: Link all object files to create the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compilation rule: Compile each .cpp file to a .o file
# This is a pattern rule that applies to all .cpp files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: Remove executable and all object files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
