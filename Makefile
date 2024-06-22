CC=gcc
BUILD_DIR=build
INCLUDE_DIR=include
SOURCES=$(wildcard src/*c)
OBJECTS=$(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
FLAGS=-I$(INCLUDE_DIR)

all: server
debug: FLAGS += -DDEBUG -g

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS)

server: $(OBJECTS)
	$(CC) -o $@ $^ $(FLAGS)

debug: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ 

clean:
	rm -rf $(BUILD_DIR)/* server debug
