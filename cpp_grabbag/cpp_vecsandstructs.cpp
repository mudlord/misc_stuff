#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>

typedef union {
	uint64_t rx; //nice unsigned types
	uint32_t ex;
	uint16_t x;
	struct {
		uint8_t lo, hi;
	} b;
} Register;

typedef struct keydata_format {
	uint32_t checksum;
	uint16_t feature_flags;
	uint16_t magic;
	uint32_t serial;
	uint32_t random;
};

typedef struct str {
	int thing1;
	bool thing2;
	float thing3;
	char thing4[50];
};

//global variables as structs
//use extern when in outside files to link to
str structie = { 0 };

void thing2(str* structs)
{
	structs->thing1 = 1;
	structs->thing2 = false;
	structs->thing3 = 0.1;
	strcpy(structs->thing4, "chocobo");
}

void thing1()
{
	str structs = { 0 };
	thing2(&structs);
	structie.thing1 = 0xFF;
}

str thing3()
{
	str structs = { 0 };
	thing2(&structs);
	return structs;
}





void vector_appendbytes(std::vector<uint8_t>& vec, uint8_t* bytes, size_t len)
{
	vec.insert(vec.end(), bytes, bytes + len);
}

void vectorsandstructs()
{
	vector<str>shit_vector;

	//zero struct and init it
	//for strings/pointers/buffers you will need to allocate and memset
	//UNLESS you statically set them
	str shit = { 0 };
	shit.thing1 = 1;
	shit.thing2 = true;
	shit.thing3 = 0.01;
	strcpy(shit.thing4, "shitfuck");
	shit_vector.push_back(shit);
	uint8_t key_buffer[sizeof(struct keydata_format)] = {0};
	//get a pointer into a bytebuffer, using a struct to set contents
	struct keydata_format* key = (struct keydata_format*)key_buffer;
	key->checksum = 0xDEADBEEF;
	key->feature_flags = 0xFFFF;
	key->magic = 0x1979;
	key->random = 0;
	key->serial = rand();

	thing1();
	str thing2 = thing3();

}

