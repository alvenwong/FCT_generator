#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <error.h>
#include <pthread.h>

#include "../common/network.h"
#include "../common/epoll.h"
#include "../common/thread_poll.h"
#include "server_configs.h"

#define MAX_FLOWS 65536

#define MAX_CPU_INDEX 23
#define MIN_CPU_INDEX 1




void* epoll_server(void* context);
static void cleanup();

struct server_configs server_conf;
unsigned int flows_size[MAX_FLOWS];
struct events_poll responce_polls[MAX_THREADS];
unsigned int efd;


#endif
