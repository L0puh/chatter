CC=gcc
BUILD_DIR=build
INCLUDE_DIR=include

SOURCES=$(wildcard src/*c)

OBJECTS=$(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
DB_OBJECTS=$(patsubst src/db/%.c, $(BUILD_DIR)/%.o, $(wildcard src/db/*c))

LIBS= -lcrypto -lpthread -lssl
FLAGS=-I$(INCLUDE_DIR) -Wno-deprecated-declarations $(LIBS) 

all: server_db 
server_db: FLAGS+=-lpq -DWITH_DB 
debug: FLAGS+=-g -DDEBUG 

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS)

$(BUILD_DIR)/%.o: src/db/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(FLAGS) 

server: $(OBJECTS) 
	$(CC) -o $@ $^ $(FLAGS) 

server_db: $(OBJECTS) $(DB_OBJECTS)
	$(CC) $(FLAGS) -o $@ $^

debug: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ 

clean:
	rm -rf $(BUILD_DIR)/* server debug server_db


