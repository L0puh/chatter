CC=gcc
BUILD_DIR=build
INCLUDE_DIR=include

SOURCES=$(wildcard src/*c)
OBJECTS=$(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
LIBS= -lcrypto -lpthread -lssl -lpq
CCFLAGS=-I$(INCLUDE_DIR) -Wno-deprecated-declarations $(LIBS) 

all: server

DEBUG ?= 1
ifeq ($(DEBUG), 1)
   CCFLAGS+=-DDEBUG -g
endif

WITH_DB ?= 1
ifeq ($(WITH_DB), 1)
	CCFLAGS+=-DWITH_DB 
endif

LOG_ON ?= 1
ifeq ($(WITH_DB), 1)
	CCFLAGS+=-LOG_ON
endif


$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CCFLAGS)

server: $(OBJECTS) 
	$(CC) -o $@ $^ $(CCFLAGS)

clean:
	rm -rf $(BUILD_DIR)/* server


