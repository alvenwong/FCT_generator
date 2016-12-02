#include "network.h"

void init_ips(struct src_ips* ips)
{
	memset(ips, 0, sizeof(struct src_ips));
}


int make_socket_nonblocking(const int sfd) 
{
	int flag;
	flag = fcntl(sfd, F_GETFL, 0);
	if (flag == -1) {
		perror("fcntl");
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(sfd, F_SETFL, flag) == -1) {
		perror("fcntl");
		return -1;
	}
	return  0;
}


int get_src_ips(struct src_ips* ips, const char* dev)
{
	assert(ips->ips_num == 0);
	if (strlen(dev) == 0) {
		printf("dev is not specified.\n");
		exit(EXIT_FAILURE);
	}

	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	char vdev[DEV_NAME_LEN];

	strcpy(ips->dev, dev);

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddr");
		exit(EXIT_FAILURE);
	}
	strcpy(vdev, dev);
	strcat(vdev, ":");	

	for (ifa=ifaddr; ifa!=NULL; ifa=ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		family = ifa->ifa_addr->sa_family;
		if (family != AF_INET || 
				(strstr(ifa->ifa_name, vdev) == NULL &&
				strcmp(ifa->ifa_name, dev) != 0)) {
			continue;
		}

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), 
						host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			perror("getnameinfo");
			exit(EXIT_FAILURE);
		}
		strcpy(ips->ips[ips->ips_num], host);
		ips->ips_num += 1;
		if (ips->ips_num > MAX_IPS) {
			break;
		}
	}
	freeifaddrs(ifaddr);

	return ips->ips_num;
}


void print_src_ips(const struct src_ips* ips)
{
	int i;
	int ips_num = ips->ips_num;

	print_split();
	printf("The device name is %s\n", ips->dev);
	if (ips_num == 0) {
		printf("None available source IPs\n");
	} else {
		printf("Client IP %s: \n", 
				ips_num <= 1 ? "is" : "are");
		for (i=0; i<ips_num; i++) {
			printf("IP %d: %s\n", i, ips->ips[i]);
		}
	}
	print_split();
}


void set_sockaddr_client(struct connection_four_tuples* conn, struct sockaddr_in* client)
{
	client->sin_family = AF_INET;
	client->sin_addr.s_addr = inet_addr(conn->saddr);
	client->sin_port = htons(conn->sport);
}


void set_sockaddr_server(struct connection_four_tuples* conn, struct sockaddr_in* server)
{
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = inet_addr(conn->daddr);
	server->sin_port = htons(conn->dport);
}

int create_connection(struct connection_four_tuples *conn)
{
	int sockfd;
	struct sockaddr_in client, server;

	set_sockaddr_client(conn, &client);
	set_sockaddr_server(conn, &server);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket error. \n");
		return EXIT_FAILURE;
	}
	if ((bind(sockfd, (struct sockaddr*)&client, sizeof(struct sockaddr_in))) < 0) {
		perror ("bind error. \n");
		return EXIT_FAILURE;
	}
	make_socket_nonblocking(sockfd);
	if (connect(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) < 0) {
		if (errno != EINPROGRESS) {
			close(sockfd);
			perror ("connect error.\n");
			return EXIT_FAILURE;
		}
	}

	return sockfd;
}


int write_request(const int fd, const int flow_size)
{
	struct request_format req;
	unsigned char payload[PAYLOAD_SIZE], *pbuf;
	int count = 0; 
	int error = 0;

	init_payload(&req);
	set_flow_size(&req, flow_size);
	pbuf = serialize_payload(payload, &req);

	count = write(fd, payload, pbuf - payload);
	if (count == -1) {
		if (errno != EINTR && errno != EAGAIN) {
			perror("write error.");
			error = -1;
		}
	} else if (count == 0) {
		// the connection has been closed
		error = 0;
	} else {
		error = count;
	}

	return error;
}


int read_request(const int fd)
{
	struct request_format req;
	unsigned char payload[PAYLOAD_SIZE];
	int count = 0;
	int error = 0;

	count = read(fd, payload, PAYLOAD_SIZE);
	if (count == -1) {
		if (errno != EAGAIN && errno != EINTR) {
			perror("read error");
			error = count;
		}			 
	} else if (count == 0) {
		error = count;
	} else {
		deserialize_payload(payload, &req);
		return req.flow_size;
	}
	return error;
}


int read_responce(const int fd, const int flow_size)
{
	int count = 0;
	int amount = 0;
	char buf[MAX_READ_BUFF];

	while (TRUE) {
		count = read(fd, buf, MAX_READ_BUFF);	
		if (count == 0) {
			perror("The connection has been closed by server.\n");
			break;
		} else if (count == -1) {
			if (errno != EAGAIN && errno != EINTR) {
				perror("read error");
				break;
			}			 
		} else {
			amount += count;
		}
		if (amount >= flow_size)
			break;
	}

	return amount;
}


void set_buffer_data(char* buf, const int size)
{
	memset(buf, 0, size);
}


int write_unit_responce(const int fd, const int size)
{
	assert(size <= MAX_WRITE_BUFF);

	char buf[MAX_WRITE_BUFF];

	set_buffer_data(buf, size);
	return write(fd, buf, size);
}



int write_responce(const int fd, const int flow_size)
{
	int count = 0;
	int left_size = flow_size;
	int amount = 0;

	while (TRUE) {
		if (left_size <= MAX_WRITE_BUFF) {
			count = write_unit_responce(fd, left_size);
			if (count <= 0) 
				return count;
			amount += count;
			break;
		} else {
			count = write_unit_responce(fd, MAX_WRITE_BUFF);
			if (count <= 0) 
				return count;
			left_size -= count;
			amount += count;
		}
	}
	return amount;
}


int create_and_bind(const unsigned int port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;
	char server[10];

	sprintf(server, "%u", port);

	memset (&hints, 0, sizeof (struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //All interface
		

	s = getaddrinfo (NULL, server, &hints, &result);
	if (s != 0) {
		perror ("getaddrinfo failed.\n");
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			break;
		}
		close (sfd);
	}
	if (rp == NULL) {
		perror ("bind failed.\n");
		return -1;
	}
	freeaddrinfo (result);

	return sfd;
}


int set_socket(const int fd)
{
	int keepalive = 1, reuseaddr = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))) {
		perror("setsockopt keepalive failed.\n");
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int))) {
		perror("setsockopt reuseaddr failed.\n");
		return -1;
	}
	if ((make_socket_nonblocking(fd)) == -1 ) {
		return -1;
	}
	return 0;
}
