CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
TARGET = main
SOURCES = main.cpp petriNet.cpp tinyxml2.cpp
OBJECTS = $(SOURCES:.cpp=.o) #write the filenames on SOURCES (familiarity) then OBJECTS automatically generate as many
all: $(TARGET) run clean
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)
run: $(TARGET)
	./$(TARGET)
.PHONY: all clean run