#ifndef CONFIGS_H
#define CONFIGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define SERVER_PORT 9001


struct server_configs {
	unsigned int port;
	bool daemon;
	bool verbose;
};


void get_configs(int argc, char *argv[], struct server_configs* conf);
void init_server_configs(struct server_configs* conf);
void read_args(int argc, char *argv[], struct server_configs* conf);
void print_server_configs(struct server_configs* conf);
void print_usage();


#endif
