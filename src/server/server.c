#define _GNU_SOURCE
#include "server.h"


void* epoll_server(void* context)
{
	int sfd, nfds;
	int i;
	struct epoll_event* events;
	int port = server_conf.port;

	if ((sfd = create_and_bind(port)) == -1) {
		exit(1);
	}
	set_socket(sfd);
	if ((listen(sfd, SOMAXCONN)) == -1) {
		exit(1);
	}
	if ((efd = epoll_create1(0)) == -1) {
		perror ("epoll_create failed.\n");
		exit(1);
	}

	add_epoll_event(efd, sfd, EPOLLIN);
	events = calloc(MAXEVENTS, sizeof(struct epoll_event));

	while (TRUE) {
		nfds = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i=0; i<nfds; i++) {
			int fd, flow_size;
			fd = events[i].data.fd;
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ) { 
					delete_epoll_event(efd, fd);
					continue;
			} else if (sfd == fd) {
					accept_epoll_events(efd, sfd);
			} else if ( events[i].events & EPOLLIN ) {
				flow_size = read_request(fd);
				if (flow_size <= 0) {
					delete_epoll_event(efd, fd);
					close(fd);
				} else {
					flows_size[fd] = flow_size;
				}
				modify_epoll_event(efd, fd, EPOLLOUT);
			} else if ( events[i].events & EPOLLOUT) {
				flow_size = flows_size[fd];
				event_push(responce_polls, fd, flow_size);
				modify_epoll_event(efd, fd, EPOLLIN);
			}
		}
	}
	free(events);
}


void* responce_thread(void* context)
{
	int index = *(int*)context;
	int fd, flow_size;
	int count;
	struct events_poll* poll = &responce_polls[index];
	struct event_info event;

	while (TRUE) {
		event_pop(poll, &event);
		fd = event.fd;
		flow_size = event.flow_size;
		count = write_responce(fd, flow_size);
		if (count != flow_size) {
			printf("Responce %d is inconsistent with flow_size %d\n", count, flow_size);
			close(fd);
		}
		//printf("thread index: %d, flow size: %d, responce: %d\n", index, flow_size, count);
		delete_epoll_event(efd, fd);
		close(fd);
		poll->flag = THREAD_IDLE;
	}
}


static void cleanup()
{

}

int main(int argc, char *argv[])
{
	int i, threads_num, rc;
	int index[MAX_THREADS];
	pthread_t resp_threads[MAX_THREADS], epoll_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	cpu_set_t cpus;

	get_configs(argc, argv, &server_conf);
	threads_num = server_conf.threads;
	assert (threads_num > 0 && threads_num < MAX_THREADS);
	init_events_polls(responce_polls, threads_num);

	for (i=0; i<threads_num; i++) {
		index[i] = i;
		CPU_ZERO(&cpus);
		CPU_SET((i+MIN_CPU_INDEX) % MAX_CPU_INDEX + MIN_CPU_INDEX, &cpus);
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
		if ((rc = pthread_create(&resp_threads[i], &attr, responce_thread, &index[i]))) {
			perror ("pthread_create");
			exit(-1);
		}
	}

	CPU_ZERO(&cpus);
	CPU_SET(0, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

	if ((rc = pthread_create(&epoll_thread, &attr, epoll_server, NULL))) {
		perror ("pthread_create");
		exit(-1);
	}
	pthread_exit(NULL);

	cleanup();

	return 0;
}
