CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -pedantic

TARGET = project
SRC = src/main.cpp

$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGET)
