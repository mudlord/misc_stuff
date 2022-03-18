
#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>


using namespace std;


class base_class {
protected:
	int x, y;
public:
	base_class() {

	};
	base_class(int x, int y) {
		this->x = x, this->y = y;
	}
	void set_values(int a, int b)
	{
		x = a; y = b;
	}
	void add(int, int);

};

class abstract_class {
protected:
	int x, y;
public:
	abstract_class() {
		x = 0; y = 1;
	}
	abstract_class(int x, int y) {
		this->x = x, this->y = y;
	}
	void set_values(int a, int b)
	{
		x = a; y = b;
	}
	void add(int x, int y)
	{
		this->x += y;
		this->y += x;

	}
	int fuck();

};

class fuckitclass : abstract_class
{
public:
	int fuck()
	{
		return 0x44;
	}

	
};

void base_class::add(int xshit, int yshit) {
	x += xshit;
	y += yshit;
}

/*
In principle, a publicly derived class inherits access to every member of a base class except:

	its constructors and its destructor
	its assignment operator members (operator=)
	its friends
	its private members
*/
/*
inherit from base class, but also have a definable class method 
which can be *overridden*/
class derived_class : public base_class {
public:
	int res()
	{
		return x * y;
	}
};


class TestClass {
	int x, y;
public:
	void add(int, int);
	int shit() { 
		return x + y; 
	}
	TestClass();
	TestClass(int, int);
};


TestClass::TestClass() {
	x = 5;
	y = 5;
}

TestClass::TestClass(int a, int b) {
	x = a;
	y = b;
}

void TestClass::add(int xshit, int yshit) {
	x += xshit;
	y += yshit;
}

void classes()
{
	fuckitclass fuckyou;
	derived_class test;
	test.set_values(0xFF, 0x90);
	int arr = test.res();
	test.add(0x33, 0x66);
	arr = test.res();
	TestClass* shit = new TestClass(0xFF, 0x90);
	shit->add(20, 40);
	delete shit;
}


typedef union {
	uint64_t rx; //nice unsigned types
	uint32_t ex;
	uint16_t x;
	struct {
		uint8_t lo, hi;
	} b;
} Register;

struct keydata_format {
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
//filesystem
void filesystems()
{
	std::filesystem::path path = std::filesystem::current_path();
	string paths = path.generic_string();
	for (auto& entry : std::filesystem::directory_iterator(path))
	{
		string str = entry.path().string();
		if (entry.is_regular_file() && entry.path().extension() == ".dll")
		{

		}
	}
}

//unique ptrs
unsigned get_filesize(FILE* fp)
{
	unsigned size = 0;
	unsigned pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return size;
}

void unique_ptrs_lambda()
{
	//the unique ptr will automatically release memory at end of scope
	auto tmp = std::make_unique<uint8_t[]>(0x10000);
	memset(tmp.get(), 0, 0x10000);

	//basically define functions inside functions!
	auto bufferlevel = []()
	{
		return double((0x8000 - (int)0x2000 /0x8000));
	};
	double buf = bufferlevel();
}

std::vector<uint8_t> load_data(const char* path, unsigned* size)
{
	auto input = unique_ptr<FILE, int (*)(FILE*)>(fopen(path, "rb"), &fclose);
	if (!input)
		return {};
	unsigned Size = get_filesize(input.get());
	*size = Size;
	std::vector<uint8_t> Memory(Size, 0);
	int res = fread((uint8_t*)Memory.data(), 1, Size, input.get());
	return Memory;
}
bool save_data(unsigned char* data, unsigned size, const char* path)
{
	auto input = unique_ptr<FILE, int (*)(FILE*)>(fopen(path, "wb"), &fclose);
	if (!input)
		return false;
	fwrite(data, 1, size, input.get());
	return true;
}
unsigned get_filesize(const char* path)
{
	//auto close/open of FILE ptrs
	auto input = unique_ptr<FILE, int (*)(FILE*)>(fopen(path, "rb"), &fclose);
	if (!input)
		return 0;
	unsigned size = get_filesize(input.get());
	return size;
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
	str shit={0};
	shit.thing1 = 1;
	shit.thing2 = true;
	shit.thing3 = 0.01;
	strcpy(shit.thing4, "shitfuck");
	shit_vector.push_back(shit);
	uint8_t key_buffer[128] = { 0 };
	//get a pointer into a bytebuffer, using a struct to set contents
	struct keydata_format* key = (struct keydata_format*)key_buffer;
	key->checksum = 0xDEADBEEF;
	key->feature_flags = 0xFFFF;
	key->magic = 0x1979;
	key->random = 0;
	key->serial = rand();

}


void MD5(BYTE* data, ULONG len, BYTE* hash_data) {
	HCRYPTPROV hProv = 0;
	HCRYPTPROV hHash = 0;
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0);
	CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	CryptHashData(hHash, data, len, 0);
	DWORD cbHash = 16;
	CryptGetHashParam(hHash, HP_HASHVAL, hash_data, &cbHash, 0);
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
}

void loops()
{
	/*
	For loops are fucking easy to understand
	just spin around up and down until a condition is met
	*/
	for (int i = 0; i < 10; i++)
	{
		//set j to 0
		int j = 0;
		j++; //post increment j
		j += 2; //add 2 to j
		j -= 2; //take away 2
		j *= 2; //multiply by 2
	}

	for (float f = 0; f < 10.0; f += 0.1)
	{

	}

	bool flag = false;
	int j = 0;
	do
	{
		//while "flag" is false, just keep swimming :3
		j++;
		if (j > 5)flag = true;


	} while (!flag);
}

void logic()
{
	//init a array of 16 bytes to nothing
	BYTE hash_bytes[16] = { 0 };
	char* name = "mudlord"; //a string
	int namelen = strlen(name); //get the length of a string
	MD5((BYTE*)name, namelen, (BYTE*)hash_bytes);
	//get a DWORD ptr (4 bytes) to data in the array
	//and byteswaps it;
	for (int i = 0; i < 4; i++) {
		DWORD* bufptr = (DWORD*)hash_bytes;
		DWORD bytes = *(DWORD*)(bufptr + i);
		*(DWORD*)(bufptr + i) = _byteswap_ulong(bytes);
	}

	//this time get by byte instead and decrement the loop
	for (int i = 16; i > 0; i--)
	{
		//get ptr
		BYTE* bufptr = hash_bytes;
		BYTE byte = bufptr[i];
		//Do an XOR this time!
		*(BYTE*)bufptr[i] = byte ^ 0x19;
	}
}



void main()
{
	srand(time(NULL));
	loops();
	logic();
	vectorsandstructs();
	classes();
	unique_ptrs_lambda();
	filesystems();
}
