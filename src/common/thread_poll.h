#ifndef THREAD_POLL_H
#define THREAD_POLL_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define MAX_THREADS 64

#define THREAD_IDLE 0
#define THREAD_BUSY 1
#define THREAD_INVALID 2

struct event_info {
	unsigned int fd;
	unsigned int flow_size;
};

struct events_poll {
	int flag;
	pthread_cond_t idle;
	pthread_cond_t busy;
	pthread_mutex_t mlock;
	struct event_info event;
};

int get_valid_thread(struct events_poll* polls);
void event_push(struct events_poll* polls, const int fd, const int flow_size);
void event_pop(struct events_poll* poll, struct event_info *event);
void init_events_poll(struct events_poll* poll, const int flag);
void init_events_polls(struct events_poll* polls, const int threads_num);


#endif
