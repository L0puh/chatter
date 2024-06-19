CC=gcc
BUILD_DIR=build
INCLUDE_DIR=include
SOURCES=$(wildcard src/*c)
OBJECTS=$(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

all: server

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< -I$(INCLUDE_DIR)

server: $(OBJECTS)
	$(CC) -o $@ $^ -I$(INCLUDE_DIR)

clean:
	rm -rf $(BUILD_DIR)/* server
