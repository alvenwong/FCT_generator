#!/usr/bin/env

import os.path
import math

PATH="raw_data/"
KEY=0
VALUE=1
UNIT_SIZE = 1448
HEADER_SIZE = 52


def get_data_from_file(filename):
	if not(os.path.isfile(filename)):
		print "file %s does not exit." % (filename)
		exit()
	dataset = dict()
	for line in open(filename).readlines():
		data = int(line.strip())
		if not(dataset.get(data)):
			dataset[data] = 1
		else:
			dataset[data] += 1
	return sorted(dataset.items(), key=lambda d:d[KEY])


def print_list(dataset):
	for ele in dataset:
		print "%s %s" % (ele[KEY], ele[VALUE]) 
	return


def get_proportion(dataset):
	proportion = list()
	tmp = list()
	product = 0
	for ele in dataset:
		#product += ele[KEY] * ele[VALUE]
		product += ele[VALUE] * int((ele[KEY] - HEADER_SIZE + UNIT_SIZE - 1) / UNIT_SIZE)
		tmp.append((ele[KEY], product))

	for ele in tmp:
		proportion.append((ele[KEY], round(ele[VALUE]*1.0/product, 4)))
	
	return proportion
	

def get_proportion_from_file(filename):
	dataset = get_data_from_file(filename)
	return get_proportion(dataset)


def get_index(prop, t):
	index_min = 0
	index_max = len(prop) - 1

	#binary search
	while (index_min <= index_max):
		index = (index_min + index_max) / 2
		if prop[index][KEY] == t:
			return index
		elif prop[index][KEY] < t:
			index_min = index + 1 
		else:
			index_max = index - 1 
	
	return index_max


def get_optimal_threshold_from_proportion(prop_a, prop_b):
	t_min = max(prop_a[0][KEY], prop_b[0][KEY])
	t_max = min(prop_a[-1][KEY], prop_b[-1][KEY])

	diff_max = 0
	optimal = t_min
	for t in range(t_min, t_max):
		index_a = get_index(prop_a, t)
		index_b = get_index(prop_b, t)
		diff = abs(prop_a[index_a][VALUE] - prop_b[index_b][VALUE])
		if diff > diff_max:
			diff_max = diff
			optimal = t

	print optimal, diff_max
	return optimal


def get_optimal_threshold_from_file(filename_a, filename_b):
	prop_a = get_proportion_from_file(fname_a)
	print_list(prop_a)
	prop_b = get_proportion_from_file(fname_b)
	get_optimal_threshold_from_proportion(prop_a, prop_b)


if __name__ == "__main__":
	fname_a = PATH + "memcached_mix_packet_size"
	fname_b = PATH + "pias_mix_packet_size"
	get_optimal_threshold_from_file(fname_a, fname_b)
