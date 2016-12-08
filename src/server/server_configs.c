#include "server_configs.h"


void get_configs(int argc, char *argv[], struct server_configs* conf)
{
	init_server_configs(conf);
	read_args(argc, argv, conf);
}



void init_server_configs(struct server_configs* conf)
{
	conf->port = SERVER_PORT;
	conf->verbose = false;
	conf->daemon = false;
}


void read_args(int argc, char *argv[], struct server_configs* conf)
{
	int result;
	opterr = 0;

	while ((result = getopt(argc, argv, "vdhp:")) != -1) {
		switch (result) {
			case 'h':
				print_usage();
				exit(0);
			case 'v':
				conf->verbose = true;
				break;
			case 'd':
				conf->daemon = true;
				break;
			case 'p':
				conf->port = atoi(optarg);
				break;
			default:
				print_usage();
				break;
		}
	}
}



void print_server_configs(struct server_configs* conf)
{
	printf("Server configuration: \n");
	printf("Port: %d\n", conf->port);
	printf("Verbose mode: %s\n", conf->verbose == false ? "False" : "True");
	printf("Daemon mode: %s\n", conf->daemon == false ? "False" : "True");
}


/* Print usage of the program */
void print_usage()
{
	printf("Usage: \n");
	printf("-p <port>   port number (default %d)\n", SERVER_PORT);
	printf("-v          give more detailed output (verbose)\n");
	printf("-d          run the server as a daemon\n");
	printf("-h          display help information\n");
}
