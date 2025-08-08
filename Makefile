CXX = g++
CXXFLAGS = -std=c++17 -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
VIS_DIR = vis
TARGET = $(BIN_DIR)/blockchain_simulator

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	@mkdir -p $(VIS_DIR)
	$(TARGET) -n 100 -T 1000 -t 100 -b 50 -w 1000 -m 25

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(VIS_DIR)

