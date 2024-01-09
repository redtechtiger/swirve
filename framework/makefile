CXX = clang++
CXXFLAGS = -g -o0 -Wall -Wextra -pthread -pedantic-errors

SRC_DIR := src
OBJ_DIR := build
BIN_DIR :=

SRC := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(SRC_DIR)/*.hpp)
OBJ := $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

.PHONY all: framework

framework: $(OBJ) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)framework $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: framework
	./framework

clean:
	rm $(wildcard build/*.o)
	rm framework
