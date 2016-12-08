CC = gcc
CFLAGS = -lm -lrt -O2 -g
LDFLAGS = -c -Wall -O2
TARGETS = client server
CLIENT_OBJS = common.o epoll.o network.o payload.o cdf.o client_configs.o client.o
SERVER_OBJS = common.o epoll.o network.o payload.o server_configs.o server.o
BIN_DIR = bin
RESULT_DIR = result
CLIENT_DIR = src/client
COMMON_DIR = src/common
SERVER_DIR = src/server

all: $(TARGETS) move

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o server

%.o: $(CLIENT_DIR)/%.c
	$(CC) $(LDFLAGS) $^ -o $@

%.o: $(COMMON_DIR)/%.c
	$(CC) $(LDFLAGS) $^ -o $@

%.o: $(SERVER_DIR)/%.c
	$(CC) $(LDFLAGS) $^ -o $@

move:
	mkdir -p $(RESULT_DIR)
	mkdir -p $(BIN_DIR)
	mv *.o $(TARGETS) $(BIN_DIR)

.PHONY: clean

clean:
	rm -rf $(BIN_DIR)/*
