CC=gcc
BUILD_DIR=build
INCLUDE_DIR=include
SOURCES=$(wildcard src/*c)
OBJECTS=$(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
LIBS= -lcrypto -lpthread -lssl
FLAGS=-I$(INCLUDE_DIR) -Wno-deprecated-declarations $(LIBS)

all: server
debug: FLAGS+=-g -DDEBUG 

$(BUILD_DIR)/%.o: src/%.c 
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS)

server: $(OBJECTS)
	$(CC) -o $@ $^ $(FLAGS)

debug: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ 

clean:
	rm -rf $(BUILD_DIR)/* server debug


