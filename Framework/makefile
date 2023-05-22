CXX = clang++
CXXFLAGS = -g -o0 -Wall -Wextra -pthread -pedantic-errors

SRC_DIR := src
OBJ_DIR := build
BIN_DIR :=

SRC := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(SRC_DIR)/*.hpp)
OBJ := $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

BUILDNUM = $(shell cat buildnumber) # Fetch current build number

.PHONY all: framework

framework: $(OBJ) $(DEPS)
	@$(eval BUILDNUM = $(shell echo $$(($(BUILDNUM)+1)))) # Calculate next build number
	@echo $(BUILDNUM) >buildnumber				   		  # Write build number

	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)framework $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: framework
	./framework

clean:
	rm $(wildcard build/*.o)
	rm framework
