CC = gcc
CFLAGS = -Wall -pthread -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Sources and Objects
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Main Target
TARGET = $(BIN_DIR)/hampod

# Tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_SRCS))

.PHONY: all clean directories tests

all: directories $(TARGET)

tests: directories $(TEST_BINS)

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile tests
$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(OBJS)
	# Compile test with all objects EXCEPT main.o (if main exists)
	# For now, we link everything since we are in Phase 0 modularity
	$(CC) $(CFLAGS) -o $@ $< $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
