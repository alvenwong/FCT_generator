#include "client_configs.h"


void get_configs(int argc, char *argv[], struct flow_configs* flow_conf, struct socket_configs* socket_conf, struct connection_four_tuples* conn)
{
	get_flow_configs(argc, argv, flow_conf);
	get_socket_configs(socket_conf, flow_conf);
	get_conn(conn, socket_conf, 0);
}


void get_conn(struct connection_four_tuples *conn, struct socket_configs* socket_conf, const int index)
{
	strcpy(conn->saddr, socket_conf->sips.ips[index]);
	strcpy(conn->daddr, socket_conf->dst_hosts[0].ip);
	conn->sport = 0;
	conn->dport = socket_conf->dst_hosts[0].port;
}


void get_flow_configs(int argc, char *argv[], struct flow_configs* flow_conf)
{
	init_flow_configs(flow_conf);
	read_args(argc, argv, flow_conf);
}


void get_socket_configs(struct socket_configs* socket_conf, struct flow_configs* flow_conf)
{
	init_socket_configs(socket_conf);
	read_socket_configs(socket_conf, flow_conf);
}



void init_flow_configs(struct flow_configs* conf)
{
	conf->flows_num = 0;
	conf->concurrent_flows = 1;
	conf->verbose = false;
	strcpy(conf->dev, "eth2");
	strcpy(conf->server, "");
	conf->port = SERVER_PORT;
	strcpy(conf->result_file, "0");
}


void read_args(int argc, char *argv[], struct flow_configs* conf)
{
	int result;
	opterr = 0;

	while ((result = getopt(argc, argv, "hvn:c:i:s:p:f:")) != -1) {
		switch (result) {
			case 'h':
				print_usage();
				exit(0);
			case 'v':
				conf->verbose = true;
				break;
			case 'n':
				conf->flows_num = atoi(optarg);
				break;
			case 'c':
				conf->concurrent_flows = atoi(optarg);
				break;
			case 'i':
				strcpy(conf->dev, optarg);
				break;
			case 's':
				strcpy(conf->server, optarg);
				break;
			case 'p':
				conf->port = atoi(optarg);
				break;
			case 'f':
				strcpy(conf->result_file, optarg);
				break;
			default:
				print_usage();
				break;
		}
	}
	if (strcmp(conf->server, "") == 0) {
		printf("Server ip is unset\n");
		exit(0);
	}
}


void init_socket_configs(struct socket_configs* socket_conf)
{
	memset(&(socket_conf->sips), 0, sizeof(struct socket_configs));	
}


void read_socket_configs(struct socket_configs* socket_conf, struct flow_configs* flow_conf)
{
	int ips_num;

	init_ips(&(socket_conf->sips));
	if ((ips_num = get_src_ips(&(socket_conf->sips), flow_conf->dev)) <= 0) {
		printf("Source IPs are unavailable.\n");
		exit(0);
	}
	if ((socket_conf->dst_num = get_dst_info(socket_conf->dst_hosts, flow_conf)) <= 0) {
		printf("Servers are unavailable.\n");
		exit(0);
	}
}


int get_dst_info(struct dst_configs dst_conf[], struct flow_configs* flow_conf)
{
	strcpy(dst_conf[0].ip, flow_conf->server);
	dst_conf[0].port = flow_conf->port;
	return 1;
}


void print_socket_configs(struct socket_configs* socket_conf)
{
	int d;

	print_split("Server information");
	print_src_ips(&(socket_conf->sips));
	for (d=0; d<socket_conf->dst_num; d++) {
		printf("Server ip: %s\n", socket_conf->dst_hosts[d].ip);
		printf("Server port: %d\n", socket_conf->dst_hosts[d].port);
	}
}


void print_flow_configs(struct flow_configs *conf)
{
	print_split("Client configuration");
	printf("Flows: %u\n", conf->flows_num);
	printf("Concurrent flows: %u\n", conf->concurrent_flows);
	printf("Device name: %s\n", conf->dev);
	printf("Verbose mode: %s\n", conf->verbose == false ? "False" : "True");
}


/* print usage of the program */
void print_usage()
{
	printf("Usage: \n");
	printf("-n <number>     number of flows (instead of -t)\n");
	printf("-c <number>     number of concurrent flows\n");
	printf("-t <time>       time in seconds (instead of -n)\n");
	printf("-v              give more detailed output (verbose)\n");
	printf("-i              device name\n");
	printf("-h              display help information\n");
	printf("-s              server ip\n");
	printf("-p              server port\n");
}
