#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "common.h"

#define PAYLOAD_SIZE 4

struct request_format {
	unsigned int flow_size;
};


void init_payload(struct request_format* req);
unsigned char* serialize_payload(unsigned char *buffer, struct request_format *req);
void deserialize_payload(unsigned char *buffer, struct request_format *req);
void set_flow_size(struct request_format* req, unsigned int flow_size);



#endif
