CC = gcc
CFLAGS = -Wall -pthread -I./include
LDFLAGS = -lhamlib -lpthread
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

.PHONY: all clean directories tests phase0_test

all: directories $(TARGET)

tests: directories $(TEST_BINS)

phase0_test: directories $(filter-out $(OBJ_DIR)/main.o $(OBJ_DIR)/main_phase0.o, $(OBJS))
	$(CC) $(CFLAGS) -o $(BIN_DIR)/phase0_test $(SRC_DIR)/main_phase0.c $(filter-out $(OBJ_DIR)/main.o $(OBJ_DIR)/main_phase0.o, $(OBJS)) $(LDFLAGS)

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Main objects - exclude main_phase0.o (it's for phase0_test only)
MAIN_OBJS = $(filter-out $(OBJ_DIR)/main_phase0.o, $(OBJS))

$(TARGET): $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile tests
$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(OBJS)
	# Compile test with all objects EXCEPT main.o (if main exists)
	# For now, we link everything since we are in Phase 0 modularity
	$(CC) $(CFLAGS) -o $@ $< $(filter-out $(OBJ_DIR)/main.o $(OBJ_DIR)/main_phase0.o, $(OBJS)) $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

