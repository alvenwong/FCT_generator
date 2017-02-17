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
#include <time.h>

#include "../common/epoll.h"
#include "../common/network.h"
#include "../common/thread_poll.h"
#include "client_configs.h"
#include "cdf.h"
#include "statistics.h"

#define MAX_FLOWS 65536
#define BACKUP    64

#define MAX_CPU_INDEX 23
#define MIN_CPU_INDEX 1

void* epoll_client(void* context);
void* request_thread(void* context);
void* shell(void* context);
int get_flow_size();
void del_flow(const int efd, const int fd);
void cleanup();

struct flow_configs flow_conf;
struct socket_configs socket_conf;
struct cdf_table req_size_dist;
struct connection_four_tuples conn;
struct flow_time_table time_table;
unsigned int flows_size[MAX_FLOWS];
struct events_poll request_polls[MAX_THREADS];
unsigned int efd;
unsigned int left_flows;
unsigned int active_flows;
pthread_mutex_t lock;
bool kill = false;
bool memaslap = false;

bool verbose_mode = false;
char cdf_filename[] = "conf/cdf"; 
char result_filename_base[] = "result/flowsize_fct_";
char result_filename[FILENAME_SIZE];


#endif
