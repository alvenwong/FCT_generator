#include "thread_poll.h"


int get_valid_thread(struct events_poll* polls)
{
	int i;
	
	for (i=0; i<MAX_THREADS; i++) {
		if (polls[i].flag == THREAD_IDLE) {
			return i;
		}
	}

	return -1;
}


void event_push(struct events_poll* polls, const int fd, const int flow_size)
{
	struct events_poll* poll = NULL;
	int index;

	if ((index = get_valid_thread(polls)) < 0) {
		printf("The flow concurrency outnumbers the threads\n");
		exit(EXIT_SUCCESS);
	}
	poll = &polls[index];
	pthread_mutex_lock(&poll->mlock);
	if (poll->flag == THREAD_BUSY) {
		pthread_cond_wait(&poll->idle, &poll->mlock);
	}
	poll->flag = THREAD_BUSY;
	poll->event.fd = fd;
	poll->event.flow_size = flow_size;
	pthread_mutex_unlock(&poll->mlock);
	pthread_cond_signal(&poll->busy);
}


void event_pop(struct events_poll* poll, struct event_info *event)
{
	pthread_mutex_lock(&poll->mlock);
	if (poll->flag == THREAD_IDLE) {
		pthread_cond_wait(&poll->busy, &poll->mlock);
	}
	event->fd = poll->event.fd;
	event->flow_size = poll->event.flow_size;
	pthread_mutex_unlock(&poll->mlock);
	pthread_cond_signal(&poll->idle);
}



void init_events_poll(struct events_poll* poll, const int flag)
{
	poll->flag = flag;
	pthread_cond_init(&poll->idle, NULL);
	pthread_cond_init(&poll->busy, NULL);
	pthread_mutex_init(&poll->mlock, NULL);
	poll->event.fd = 0;
	poll->event.flow_size = 0;
}


void init_events_polls(struct events_poll* polls, const int threads_num)
{
	int i;
	for (i=0; i<threads_num; i++) {
		init_events_poll(&polls[i], THREAD_IDLE);		
	}

	for (; i<MAX_THREADS; i++) {
		init_events_poll(&polls[i], THREAD_INVALID);		
	}
}
