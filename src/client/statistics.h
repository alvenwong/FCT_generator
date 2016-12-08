#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include "../common/common.h"

#define S_TO_US 1000000


struct flow_time {
	int fd;
	struct timeval req_time;
	struct timeval resp_time;
	int flow_size;
	int fct;
};


struct flow_time_table {
	int max_entries;
	struct flow_time** flows_time;
};


int init_flow_time_table(struct flow_time_table *time_table, const int max_entries);
void del_flow_time_table(struct flow_time_table *time_table);
void del_flow_time_entry(struct flow_time_table *time_table, const int fd);
int add_flow_time_entry(struct flow_time_table *time_table, const int fd, const int flow_size);
void set_req_time(struct flow_time_table *time_table, const int fd);
int get_time_diff(struct timeval *start, struct timeval *end);
void set_resp_time(struct flow_time_table *time_table, const int fd);
int set_flow_time_flowsize(struct flow_time_table *time_table, const int fd, const int flow_size);
void print_statistics(struct flow_time_table *time_table);


#endif

