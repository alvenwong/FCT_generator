#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"
#include "payload.h"

#define DEBUG
#define DEV_NAME_LEN 16
#define MAX_IPS 16
#define MAX_BUFF_SIZE (1 << 16)

#define MAX_READ_BUFF MAX_BUFF_SIZE
#define MAX_WRITE_BUFF MAX_BUFF_SIZE


struct connection_four_tuples {
	char saddr[NI_MAXHOST];
	char daddr[NI_MAXHOST];
	unsigned int sport;
	unsigned int dport;
};

struct src_ips {
	char dev[DEV_NAME_LEN];
	char ips[MAX_IPS][NI_MAXHOST];
	unsigned int ips_num;
};


void init_ips(struct src_ips* ips);
int make_socket_nonblocking(const int sfd);
int get_src_ips(struct src_ips* ips, const char *dev);
void print_src_ips(const struct src_ips* ips);
void set_sockaddr_client(struct connection_four_tuples* conn, struct sockaddr_in* client);
void set_sockaddr_server(struct connection_four_tuples* conn, struct sockaddr_in* client);
int create_connection(struct connection_four_tuples *conn);
int read_request(const int fd);
int write_request(const int fd, const int flow_size);
int read_responce(const int fd, const int flow_size);
int write_unit_responce(const int fd, const int size);
int write_responce(const int fd, const int flow_size);
int create_and_bind(const unsigned int port);
int set_socket(const int fd);

#endif
