#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include "../common/common.h"

#define S_TO_US 1000000
#define THRES_NUM 3

struct flow_time {
	int fd;
	struct timeval req_time;
	struct timeval resp_time;
	int flow_size;
	int fct;
};

struct stat_result {
	unsigned int threshold;
	int counter;
	int avg_flow_size;
	int avg_fct;
	long long int flow_size_tmp;
	long long int fct_tmp;
};


struct flow_time_table {
	int max_entries;
	struct flow_time** flows_time;
	unsigned int thresholds[THRES_NUM];
	struct stat_result results[THRES_NUM];
};


int init_flow_time_table(struct flow_time_table *time_table, const int max_entries);
void del_flow_time_table(struct flow_time_table *time_table);
void del_flow_time_entry(struct flow_time_table *time_table, const int fd);
int add_flow_time_entry(struct flow_time_table *time_table, const int fd, const int flow_size);
void set_req_time(struct flow_time_table *time_table, const int fd);
int get_time_diff(struct timeval *start, struct timeval *end);
void set_resp_time(struct flow_time_table *time_table, const int fd);
int set_flow_time_flowsize(struct flow_time_table *time_table, const int fd, const int flow_size);
void print_statistics(struct flow_time_table *time_table, char* filename);


#endif

