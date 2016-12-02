#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <error.h>

#include "../common/network.h"
#include "../common/epoll.h"
#include "server_configs.h"

#define MAX_FLOWS 65536

static void epoll_server(const unsigned int port);
static void cleanup();

struct server_configs server_conf;
unsigned int flows_size[MAX_FLOWS];

#endif
