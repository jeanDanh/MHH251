CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
TARGET = pnmlParser
SOURCES = pnmlParser.cpp tinyxml2.cpp
OBJECTS = $(SOURCES:.cpp=.o) #write the filenames on SOURCES (familiarity) then OBJECTS automatically generate as many
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
%.o: %.cpp tinyxml2.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)
.PHONY: all clean