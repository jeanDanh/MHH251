CXX = g++
CUDD_DIR = lib/cudd-cudd-3.0.0/install
OR_TOOLS_DIR = lib/or-tools

INCLUDES = -I$(CUDD_DIR)/include -I$(OR_TOOLS_DIR)/include

CXXFLAGS = -std=c++17 -Wall -Wextra -g -fPIC $(INCLUDES)

LDFLAGS = -L$(CUDD_DIR)/lib -L$(OR_TOOLS_DIR)/lib \
          -lcudd \
          -lortools \
          -lprotobuf \
          -lz -lrt -lpthread \
          -Wl,-rpath,'$$ORIGIN/$(CUDD_DIR)/lib' \
          -Wl,-rpath,'$$ORIGIN/$(OR_TOOLS_DIR)/lib'

TARGET_TASK1 = task1
TARGET_TASK3 = task3
TARGET_TASK4 = task4

SOURCES_TASK1 = main.cpp petriNet.cpp tinyxml2.cpp
SOURCES_TASK3 = main.cpp petriNet.cpp symbolicPetriNet.cpp tinyxml2.cpp
SOURCES_TASK4 = test_task4.cpp deadlockDetector.cpp petriNet.cpp symbolicPetriNet.cpp tinyxml2.cpp

OBJECTS_TASK1 = $(SOURCES_TASK1:.cpp=.o)
OBJECTS_TASK3 = $(SOURCES_TASK3:.cpp=.o)
OBJECTS_TASK4 = $(SOURCES_TASK4:.cpp=.o)

# COMMANDS
all: $(TARGET_TASK3) $(TARGET_TASK4) run3 clean

task1: $(OBJECTS_TASK1)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TASK1) $(OBJECTS_TASK1)
run1: task1
	./$(TARGET_TASK1)

task3: $(OBJECTS_TASK3)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TASK3) $(OBJECTS_TASK3) $(LDFLAGS)
run3: task3
	./$(TARGET_TASK3)

task4: $(OBJECTS_TASK4)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TASK4) $(OBJECTS_TASK4) $(LDFLAGS)
run4: task4
	./$(TARGET_TASK4)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS_TASK1) $(OBJECTS_TASK3) $(OBJECTS_TASK4) $(TARGET_TASK1) $(TARGET_TASK3) $(TARGET_TASK4)

.PHONY: all task1 task3 task4 run1 run3 run4 clean