CXX = g++

CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude

TARGET = deadproc

SOURCES = src/main.cpp \
          src/Process.cpp \
          src/ProcessScanner.cpp

OBJECTS = $(SOURCES:.cpp=.o)


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)


src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	rm -f $(OBJECTS) $(TARGET)


run: $(TARGET)
	./$(TARGET)


.PHONY: all clean run