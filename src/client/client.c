#define _GNU_SOURCE
#include "client.h"


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
				delete_epoll_event(efd, fd);
				continue;
			} else if (events[i].events & EPOLLOUT) {
				int error;
				if (active_flows < concurrent_flows) {	
					//flow_size = get_flow_size();
					flow_size = 10240;	
					flows_size[fd] = flow_size;
					write_request(fd, flow_size);
					if ((error = modify_epoll_event(efd, fd, EPOLLIN)) < 0) {
						perror ("modify_epoll_event");
						delete_epoll_event(efd, fd);
						close(fd);
					} else {
						active_flows += 1;
					}
				} else {
					if ((error = modify_epoll_event(efd, fd, EPOLLOUT)) < 0) {
						perror ("modify_epoll_event");
						delete_epoll_event(efd, fd);
						close(fd);
					}
				}
			} else if (events[i].events & EPOLLIN) {
				int count;
				flow_size = flows_size[fd];
				printf("flow size: %d\n", flow_size);
				count = read_responce(fd, flow_size);
				if (count != flow_size) {
					//printf("responce is smaller than expected!\n");
				} else {
					printf("success!\n");
				}
				active_flows -= 1;
				left_flows -= 1;
				delete_epoll_event(efd, fd);
				close(fd);
			}
		}
	}
	
	free(events);
}


void get_conn(struct connection_four_tuples *conn, struct socket_configs* socket_conf)
{
	strcpy(conn->saddr, "172.16.32.207");
	strcpy(conn->daddr, socket_conf->dst_hosts[0].ip);
	conn->sport = 0;
	conn->dport = socket_conf->dst_hosts[0].port;
}


void epoll_client()
{
	int efd;

	efd = create_epoll();
	epoll_loop(efd);

	return;
}



void print_statistic()
{
	return;

}

void cleanup()
{
	return;
}



int main(int argc, char *argv[])
{
	get_configs(argc, argv, &flow_conf, &socket_conf);
	get_cdf(&req_size_dist, cdf_filename);
	get_conn(&conn, &socket_conf);

	print_cdf(&req_size_dist);
	print_flow_configs(&flow_conf);
	print_socket_configs(&socket_conf);

	epoll_client();	
	print_statistic();
	cleanup();

	return 0;
}
