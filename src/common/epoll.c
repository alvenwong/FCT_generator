#include "epoll.h"

int create_epoll()
{
	int efd;
	efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create");
		exit(1);
	}
	return efd;
}


int create_epoll_event(struct connection_four_tuples *conn, const int efd)
{
	int fd;
	if ((fd = create_connection(conn)) != EXIT_FAILURE) {
		add_epoll_event(efd, fd, EPOLLOUT);
		return fd;
	}

	return EXIT_FAILURE;
}


int create_N_epoll_events(struct connection_four_tuples *conn, const int efd, const int num)
{
	int n, i;
	int error;
	int result = 0;

	n = (conn->sport == 0) ? num : 1;

	for (i=0; i<n; i++) {
		if ((error = create_epoll_event(conn, efd)) != EXIT_FAILURE)
			result += 1;
		if (i % PAUSE_NUM == 0)
			sleep(1);
	}

	return result;
}


int add_epoll_event(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	event.data.fd = fd;
	assert(events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event)) == -1) {
		perror ("epoll ctl: add_epoll_event");
		return EXIT_FAILURE;
	} 
	return EXIT_SUCCESS;
}


int modify_epoll_event(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	event.data.fd = fd;
	assert (events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_MOD, fd, &event)) == -1) {
		perror ("epoll ctl: modify_epoll_event");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int delete_epoll_event(const int efd, const int fd)
{
	struct epoll_event event;
	if ((epoll_ctl (efd, EPOLL_CTL_DEL, fd, &event)) == -1) {
		perror ("epoll_ctl: delete_epoll_event");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


void accept_epoll_events(const int efd, const int sfd)
{
	struct sockaddr in_addr;
	socklen_t in_len;
	int infd;

	while(TRUE) {
		in_len = sizeof (in_addr);
		infd = accept (sfd, &in_addr, &in_len);
		if (infd == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				//we have processed all incomming connectings
				break;
			} else {
				perror ("accept");
				break;
			}
		}
#ifdef DEBUG
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		int s;
		s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), 
						sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
		if (s == 0)	
			printf("Accept connection on description %d, host=%s, port=%s\n",
					infd, hbuf, sbuf);
#endif
		//make the incoming socket non-blocking 
		//and add it to the list of fds to monitor
		set_socket(infd);
		add_epoll_event(efd, infd, EPOLLIN);
	}
}
