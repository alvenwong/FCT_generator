#define _GNU_SOURCE
#include "client.h"


int get_flow_size()
{
	return gen_random_cdf(&req_size_dist);
}


void del_flow(const int efd, const int fd)
{
	delete_epoll_event(efd, fd);
	close(fd);
	del_flow_time_entry(&time_table, fd);
}


void* shell(void* context)
{
	int number = *(int*)context;
	char command[100];

	while (!memaslap) {
		sleep(1);
	};

	sprintf(command, "sh memcached.sh %d &", number);
	printf("%s\n", command);
	system(command);

	return NULL;
}


void* epoll_client(void* context)
{
	struct epoll_event* events;
	int flows_num = flow_conf.flows_num;
	int concurrent_flows = flow_conf.concurrent_flows;
	int nfds;
	int i;

	efd = create_epoll();
	left_flows = create_N_epoll_events(&conn, efd, flows_num);
	active_flows = 0;

	events = calloc(MAXEVENTS, sizeof(struct epoll_event));
	while (left_flows > 0) {
		if (!memaslap && left_flows < flows_num * 0.7) {
			memaslap = true;
		}

		nfds = epoll_wait(efd, events, MAXEVENTS, 0);
		for (i=0; i<nfds; i++) {
			int fd;
			int flow_size;
			fd = events[i].data.fd;
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP)) {
				del_flow(efd, fd);
				continue;
			} else if (events[i].events & EPOLLOUT) {
				int error;
				if (active_flows < concurrent_flows) {	
					flow_size = get_flow_size();
					flows_size[fd] = flow_size;
					write_request(fd, flow_size);
					if ((error = modify_epoll_event(efd, fd, EPOLLIN)) < 0) {
						perror ("modify_epoll_event");
						del_flow(efd, fd);
					} else {
						active_flows += 1;
						add_flow_time_entry(&time_table, fd, flow_size);
					}
					event_push(request_polls, fd, flow_size);
				} else {
					if ((error = modify_epoll_event(efd, fd, EPOLLOUT)) < 0) {
						perror ("modify_epoll_event");
						del_flow(efd, fd);
					}
				}
			} else if (events[i].events & EPOLLIN) {
				continue;
			} 
		}
	}
	free(events);

	string_joint(result_filename_base, flow_conf.result_file, result_filename);
	print_statistics(&time_table, result_filename);
	kill = true;

	exit(EXIT_SUCCESS);
}


void* request_thread(void* context)
{
	int index = *(int*)context;
	int fd, flow_size;
	int count;
	struct events_poll* poll = &request_polls[index];
	struct event_info event;
	
	while (!kill) {
		event_pop(poll, &event);
		fd = event.fd;
		flow_size = event.flow_size;
		count = read_responce(fd, flow_size);
		if (count == -1) {
			incre_error_flows(&time_table, count);
			//delete_epoll_event(efd, fd);
			del_flow_time_entry(&time_table, fd);
		} else if (count != flow_size ) {
			incre_error_flows(&time_table, count);
			del_flow(efd, fd);
		} else {
			set_resp_time(&time_table, fd);
			delete_epoll_event(efd, fd);
			close(fd);
		}
		pthread_mutex_lock(&lock);
		active_flows -= 1;
		left_flows -= 1;
		pthread_mutex_unlock(&lock);
		poll->flag = THREAD_IDLE;
	}
	printf("request thread ends!\n");
	
	return NULL;
}


void cleanup()
{
	del_flow_time_table(&time_table);
}


int main(int argc, char *argv[])
{
	int i, rc;
	int memaslap_num, threads_num;
	int index[MAX_THREADS];
	pthread_t req_threads[MAX_THREADS], epoll_thread, shell_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	cpu_set_t cpus;
	
	get_configs(argc, argv, &flow_conf, &socket_conf, &conn);
	get_cdf(&req_size_dist, cdf_filename);
	init_flow_time_table(&time_table, flow_conf.flows_num + BACKUP);

	threads_num = flow_conf.threads;
	memaslap_num = flow_conf.memaslap;
	assert (threads_num > 0 && threads_num < MAX_THREADS);
	init_events_polls(request_polls, threads_num);
	print_cdf(&req_size_dist);
	print_flow_configs(&flow_conf);
	print_socket_configs(&socket_conf);

	for (i=0; i<threads_num; i++) {
		index[i] = i;
		CPU_ZERO(&cpus);
		CPU_SET((i+MIN_CPU_INDEX) % MAX_CPU_INDEX + MIN_CPU_INDEX, &cpus);
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
		if ((rc = pthread_create(&req_threads[i], &attr, request_thread, &index[i]))) {
			perror ("pthread_create");
			exit(-1);
		}
	}

	CPU_ZERO(&cpus);
	CPU_SET(0, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	if ((rc = pthread_create(&epoll_thread, &attr, epoll_client, NULL))) {
		perror ("pthread_create");
		exit(-1);
	}
#ifdef MEMCACHED
	CPU_ZERO(&cpus);
	CPU_SET(23, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	if ((rc = pthread_create(&shell_thread, &attr, shell, &memaslap_num))) {
		perror ("pthread_create");
		exit(-1);
	}
#endif
	pthread_exit(NULL);

	cleanup();

	return 0;
}
