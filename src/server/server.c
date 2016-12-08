#include "server.h"

static void epoll_server(const unsigned int port)
{
	int sfd, efd, nfds;
	int i;
	struct epoll_event* events;

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
				printf("fd: %d, in flow size: %d\n", fd, flow_size);
				if (flow_size <= 0) {
					delete_epoll_event(efd, fd);
					close(fd);
				} else {
					flows_size[fd] = flow_size;
				}
				modify_epoll_event(efd, fd, EPOLLOUT);
			} else if ( events[i].events & EPOLLOUT) {
				int count = 0;
				flow_size = flows_size[fd];
				count = write_responce(fd, flow_size);
				if (count != flow_size) {
					close(fd);
				}
				delete_epoll_event(efd, fd);
			}
		}
	}
	free(events);

}


static void cleanup()
{

}

int main(int argc, char *argv[])
{
	get_configs(argc, argv, &server_conf);

	epoll_server(server_conf.port);

	cleanup();

	return 0;
}
