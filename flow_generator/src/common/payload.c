#include "payload.h"

void init_payload(struct request_format* req)
{
	req->flow_size = 0;
}


void set_flow_size(struct request_format* req, unsigned int flow_size)
{
	req->flow_size = flow_size;
}


unsigned char* serialize_payload(unsigned char *buffer, struct request_format *req)
{
	return serialize_u32(buffer, req->flow_size);
}

void deserialize_payload(unsigned char *buffer, struct request_format *req)
{
	unsigned char* tmp_buf = buffer;
	req->flow_size = deserialize_u32(tmp_buf);
}
