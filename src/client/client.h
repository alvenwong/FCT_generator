#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include <unistd.h>
#include <ifaddrs.h>
#include <assert.h>
#include <errno.h>

#include "../common/epoll.h"
#include "../common/network.h"
#include "client_configs.h"
#include "cdf.h"

#define MAX_FLOWS 65536


void epoll_loop(const int efd);
void epoll_client();
void print_statistic();
void cleanup();

struct flow_configs flow_conf;
struct socket_configs socket_conf;
struct cdf_table req_size_dist;
struct connection_four_tuples conn;
unsigned int flows_size[MAX_FLOWS];

bool verbose_mode = false;
char cdf_filename[] = "conf/cdf"; 


#endif
