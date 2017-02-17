#ifndef MY_EPOLL_H
#define MY_EPOLL_H

#include <sys/epoll.h>

#include "network.h"

//#define DEBUG
#define MAXEVENTS 60000

int create_epoll();
int create_epoll_event(struct connection_four_tuples *conn, const int efd);
int create_N_epoll_events(struct connection_four_tuples *conn, const int efd, const int num);
int add_epoll_event(const int fd, const int efd, const int events);
int modify_epoll_event(const int fd, const int efd, const int events);
int delete_epoll_event(const int fd, const int efd);
void accept_epoll_events(const int efd, const int sfd);


#endif
