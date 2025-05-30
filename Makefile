CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17

SRC_DIR = .
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = $(BIN_DIR)/main_program

all: $(TARGET)

$(BIN_DIR):
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

clean:
	if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR)