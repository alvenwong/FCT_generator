#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define SUCCESS 0
#define FAILURE -1
#define FILENAME_SIZE 128


void print_split(const char* str);

unsigned char * serialize_u64(unsigned char *buffer, uint64_t value);
unsigned char * serialize_u32(unsigned char *buffer, uint32_t value);
unsigned char * serialize_u16(unsigned char *buffer, uint16_t value);
unsigned char * serialize_u8(unsigned char *buffer, uint8_t value);

uint8_t deserialize_u8(unsigned char *buffer);
uint16_t deserialize_u16(unsigned char *buffer);
uint32_t deserialize_u32(unsigned char *buffer);
uint64_t deserialize_u64(unsigned char *buffer);

#endif 
