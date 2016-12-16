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

	time_table->thresholds[0] = 100 * 1000;
	time_table->thresholds[1] = 10 * 1000 * 1000;
	time_table->thresholds[2] = 0xffffffff;

	for (i=0; i<THRES_NUM; i++) {
		memset(&time_table->results[i],
				0, 
				sizeof(struct stat_result));
		time_table->results[i].threshold = time_table->thresholds[i];
	}

	time_table->error.total_flows = max_entries;
	time_table->error.size_error = 0;
	time_table->error.connection_error = 0;

	return SUCCESS;
}


void incre_error_flows(struct flow_time_table* time_table, const int error)
{
	if (error == CONNECTION_ERROR) {
		time_table->error.connection_error += 1;
	} else {
		time_table->error.size_error += 1;
	}
}


void del_flow_time_entry(struct flow_time_table* time_table, const int fd)
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


int get_group_index(const unsigned int* thres, const int flow_size)
{
	int i;
	for (i=0; i<THRES_NUM - 1; i++) {
		if (flow_size < thres[i])
			return i;
	}

	return i;
}


void print_results(struct flow_time_table *time_table)
{
	print_split("Flow Completion time (FCT) results");
	if (time_table->error.size_error > 0) {
		printf("%d flows occur size error.\n", time_table->error.size_error);
	}
	if (time_table->error.connection_error > 0) {
		printf("%d flows occur connection error.\n", time_table->error.connection_error);
	}
	printf("%d flows among (0, 100K), average FCT is %d\n", 
			time_table->results[0].counter,
			time_table->results[0].avg_fct);
	printf("%d flows among [100K, 10M), average FCT is %d\n", 
			time_table->results[1].counter,
			time_table->results[1].avg_fct);
	printf("%d flows among [10M, ), average FCT is %d\n", 
			time_table->results[2].counter,
			time_table->results[2].avg_fct);
}


void print_statistics(struct flow_time_table *time_table, char* filename)
{
	int entries = time_table->max_entries;
	int i, count;
	int verbose = 0;
	int flow_size, fct, index;
	FILE *fp = NULL;

	if (strcmp(filename, "") == 0) {
		verbose = 1;
		print_split("Flow size and FCT");
	} else {
		if ((fp = fopen(filename, "w")) == NULL) {
			printf("Open %s failed.\n", filename);
			exit(EXIT_SUCCESS);
		}
	}

	for (i=0, count=0; i<entries; i++) {
		if (time_table->flows_time[i]->fd == 0)
			continue;

		flow_size = time_table->flows_time[i]->flow_size;
		fct = time_table->flows_time[i]->fct;
		if (verbose) {
			printf("%d: [%d, %d]\n", count++, flow_size, fct);
		} else {
			fprintf(fp, "%d %d\n", flow_size, fct);
		}

		index = get_group_index(time_table->thresholds, flow_size);
		time_table->results[index].counter += 1;
		time_table->results[index].flow_size_tmp += flow_size;
		time_table->results[index].fct_tmp += fct;
	}

	if (verbose == 0)
		fclose(fp);

	for (i=0; i<THRES_NUM; i++) {
		struct stat_result* result = &time_table->results[i];
		if (result->counter > 0) {
			result->avg_flow_size = result->flow_size_tmp / result->counter;
			result->avg_fct = result->fct_tmp / result->counter;
		}
	}
	
	print_results(time_table);
}
