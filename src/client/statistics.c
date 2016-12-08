#include "statistics.h"

int init_flow_time_table(struct flow_time_table *time_table, const int max_entries)
{
	int i;
	time_table->max_entries = max_entries;
	if ((time_table->flows_time = (struct flow_time**)calloc(max_entries, sizeof(struct flow_time*))) == NULL) {
		free(time_table->flows_time);
		return FAILURE;
	}

	for (i=0; i<max_entries; i++) {
		if ((time_table->flows_time[i] = (struct flow_time*)calloc(1, sizeof(struct flow_time))) == NULL) {
			del_flow_time_table(time_table);
			return FAILURE;
		}
	}

	return SUCCESS;
}


void del_flow_time_entry(struct flow_time_table *time_table, const int fd)
{
	time_table->flows_time[fd]->fd = 0;
}


int add_flow_time_entry(struct flow_time_table *time_table, const int fd, const int flow_size)
{
	int result; 
	result = set_flow_time_flowsize(time_table, fd, flow_size);
	set_req_time(time_table, fd);

	return result;
}


void del_flow_time_table(struct flow_time_table *time_table)
{
	int max_entries = time_table->max_entries;
	int i;
	for (i=0; i<max_entries; i++) {
		free(time_table->flows_time[i]);
	}

	free(time_table->flows_time);
}


void set_req_time(struct flow_time_table *time_table, const int fd)
{
	assert (time_table->flows_time[fd]->fd != 0);
	gettimeofday(&time_table->flows_time[fd]->req_time, NULL);
}


int get_time_diff(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * S_TO_US + (end->tv_usec - start->tv_usec);
}


void set_resp_time(struct flow_time_table *time_table, const int fd)
{
	assert (time_table->flows_time[fd]->fd != 0);
	gettimeofday(&time_table->flows_time[fd]->resp_time, NULL);
	time_table->flows_time[fd]->fct = get_time_diff(
			&time_table->flows_time[fd]->req_time,
			&time_table->flows_time[fd]->resp_time);
}


int set_flow_time_flowsize(struct flow_time_table *time_table, const int fd, const int flow_size)
{
	int entries = time_table->max_entries;
	int i;
	assert (time_table->flows_time[fd]->fd == 0);
	if (fd >= entries) {
		// double the memory
		time_table->max_entries *= 2;
		if ((time_table->flows_time = (struct flow_time**)realloc(
						time_table->flows_time, 
						time_table->max_entries*sizeof(struct flow_time*))) == NULL) {
			del_flow_time_table(time_table);
			return FAILURE;
		}

		for (i=entries; i<time_table->max_entries; i++) {
			if ((time_table->flows_time[i] = (struct flow_time*)calloc(1, sizeof(struct flow_time))) == NULL) {
				del_flow_time_table(time_table);
				return FAILURE;
			}
		}
	}
	time_table->flows_time[fd]->fd = fd;
	time_table->flows_time[fd]->flow_size = flow_size;

	return SUCCESS;
}


void print_statistics(struct flow_time_table *time_table)
{
	int entries = time_table->max_entries;
	int i, count;
	
	print_split();
	for (i=0, count=0; i<entries; i++) {
		if (time_table->flows_time[i]->fd == 0)
			continue;
		printf("%d: [%d, %d]\n", 
				count++, 
				time_table->flows_time[i]->flow_size,
				time_table->flows_time[i]->fct);
	}
}
