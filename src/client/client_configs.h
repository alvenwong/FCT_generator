#ifndef CLIENT_CONFIGS_H
#define CLIENT_CONFIGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "../common/network.h"

#define MAX_CLIENT_IPS MAX_IPS
#define MAX_SERVER_IPS MAX_IPS
#define SERVER_PORT 9001

struct flow_configs {
	unsigned int flows_num;
	unsigned int concurrent_flows;
	char dev[DEV_NAME_LEN];
	bool verbose;
	char server[NI_MAXHOST];
	unsigned int port;
	char result_file[FILENAME_SIZE];
};

struct dst_configs {
	char ip[NI_MAXHOST];
	unsigned int port;
};

struct socket_configs {
	struct src_ips sips;
	struct dst_configs dst_hosts[MAX_SERVER_IPS];
	unsigned int dst_num;
};


void get_configs(int argc, char *argv[], struct flow_configs* flow_conf, struct socket_configs* socket_conf, struct connection_four_tuples* conn);
void get_flow_configs(int argc, char *argv[], struct flow_configs* flow_conf);
void get_socket_configs(struct socket_configs* socket_conf, struct flow_configs* flow_conf);
void get_conn(struct connection_four_tuples *conn, struct socket_configs* socket_conf, const int index);
void init_flow_configs(struct flow_configs* conf);
void init_socket_configs(struct socket_configs* socket_conf);
void read_socket_configs(struct socket_configs* socket_conf, struct flow_configs* flow_conf);
int get_dst_info(struct dst_configs dst_conf[], struct flow_configs* flow_conf);
void print_socket_configs(struct socket_configs* socket_conf);
void print_usage();
void read_args(int argc, char *argv[], struct flow_configs *conf);
void init_configs(struct flow_configs *conf);
void print_flow_configs(struct flow_configs *conf);


#endif
