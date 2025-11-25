# VARIABLES 
CXX = g++
CUDD_DIR = lib/cudd-cudd-3.0.0/install
INCLUDES = -I$(CUDD_DIR)/include
CXXFLAGS = -std=c++11 -Wall -Wextra -g $(INCLUDES)
LDFLAGS = -L$(CUDD_DIR)/lib -lcudd -Wl,-rpath,$(CUDD_DIR)/lib
TARGET_TASK1 = task1
TARGET_TASK3 = task3
SOURCES_TASK1 = main.cpp petriNet.cpp tinyxml2.cpp
SOURCES_TASK3 = main.cpp petriNet.cpp symbolicPetriNet.cpp tinyxml2.cpp
OBJECTS_TASK1 = $(SOURCES_TASK1:.cpp=.o)
OBJECTS_TASK3 = $(SOURCES_TASK3:.cpp=.o)

# COMMANDS
all: $(TARGET_TASK3) run3 clean
task1: $(OBJECTS_TASK1)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TASK1) $(OBJECTS_TASK1)
run1: task1
	./$(TARGET_TASK1)
task3: $(OBJECTS_TASK3)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TASK3) $(OBJECTS_TASK3) $(LDFLAGS)
run3: task3
	./$(TARGET_TASK3)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS_TASK1) $(OBJECTS_TASK3) $(TARGET_TASK1) $(TARGET_TASK3)
.PHONY: all task1 task3 run1 run3 clean