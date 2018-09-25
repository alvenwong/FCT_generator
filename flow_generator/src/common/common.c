#include "common.h"


void __print_split()
{
	printf("===========================\n");
}


void print_split(const char* str)
{
	__print_split();
	if (strcmp(str, "") == 0) 
		return;

	printf("%s\n", str);
	__print_split();
}


void string_joint(char* str1, char* str2, char* dest)
{
	strcpy(dest, str1);
	strcat(dest, str2);
}


unsigned char * serialize_u64(unsigned char *buffer, uint64_t value)
{
    int bits = 8;
    int bit = 0;
    for (; bit<bits; bit++) {
		buffer[bit] = value >> (7-bit)*8;
    }
    return buffer + 8;
}

unsigned char * serialize_u32(unsigned char *buffer, uint32_t value)
{
    int bits = 4;
    int bit = 0;
    for (; bit<bits; bit++) {
		buffer[bit] = value >> (3-bit)*8;
    }
    return buffer + 4;
}

unsigned char * serialize_u16(unsigned char *buffer, uint16_t value)
{
    int bits = 2;
    int bit = 0;
    for (; bit<bits; bit++) {
		buffer[bit] = value >> (1-bit)*8;
    }
    return buffer + 2;
}


unsigned char * serialize_u8(unsigned char *buffer, uint8_t value)
{
	buffer[0] = value;
	return buffer + 1;
}


uint8_t deserialize_u8(unsigned char *buffer)
{
	return buffer[0];
}

uint16_t deserialize_u16(unsigned char *buffer)
{
    uint16_t value=0;
    int i=0;
    for (; i<2; i++) {
		value = (value << 8) + buffer[i];
    }
    return value;
}

uint32_t deserialize_u32(unsigned char *buffer)
{
    uint32_t value=0;
    int i=0;
    for (; i<4; i++) {
		value = (value << 8) + buffer[i];
    }
    return value;
}

uint64_t deserialize_u64(unsigned char *buffer)
{
    uint64_t value=0;
    int i=0;
    for (; i<8; i++) {
		value = (value << 8) + buffer[i];
    }
    return value;
}
