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


void epoll_loop(const int efd)
{
	struct epoll_event* events;
	int flows_num = flow_conf.flows_num;
	int left_flows = create_N_epoll_events(&conn, efd, flows_num);
	int concurrent_flows = flow_conf.concurrent_flows;
	int active_flows = 0;
	int nfds;
	int i;

	events = calloc(MAXEVENTS, sizeof(struct epoll_event));
	while (TRUE) {
		if (left_flows <= 0)
			break;
		
		nfds = epoll_wait(efd, events, MAXEVENTS, -1);
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
				} else {
					if ((error = modify_epoll_event(efd, fd, EPOLLOUT)) < 0) {
						perror ("modify_epoll_event");
						del_flow(efd, fd);
					}
				}
			} else if (events[i].events & EPOLLIN) {
				int count, error;
				flow_size = flows_size[fd];
				count = read_responce(fd, flow_size);
				if (count != flow_size) {
					printf("responce %d is inconsistent with expected %d!\n", count, flow_size);
				} else {
					set_resp_time(&time_table, fd);
				}
				active_flows -= 1;
				left_flows -= 1;
				if ((error = modify_epoll_event(efd, fd, EPOLLIN)) < 0) {
					perror ("modify_epoll_event");
					del_flow(efd, fd);
				}
				delete_epoll_event(efd, fd);
				close(fd);
			}
		}
	}

	free(events);
}


void epoll_client()
{
	int efd;

	efd = create_epoll();
	epoll_loop(efd);

	return;
}


void cleanup()
{
	del_flow_time_table(&time_table);
}

void string_joint(char* str1, char* str2, char* dest)
{
	strcpy(dest, str1);
	strcat(dest, str2);
}

int main(int argc, char *argv[])
{
	get_configs(argc, argv, &flow_conf, &socket_conf, &conn);
	get_cdf(&req_size_dist, cdf_filename);
	init_flow_time_table(&time_table, flow_conf.flows_num + BACKUP);

	print_cdf(&req_size_dist);
	print_flow_configs(&flow_conf);
	print_socket_configs(&socket_conf);

	epoll_client();	
	
	string_joint(result_filename_base, flow_conf.result_file, result_filename);
	print_statistics(&time_table, result_filename);
	cleanup();

	return 0;
}
