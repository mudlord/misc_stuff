
#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>


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
	virtual int fuck() = 0;

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
	abstract_class *fuc1 = (abstract_class*) & fuckyou;
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

typedef struct str {
	int thing1;
	bool thing2;
	float thing3;
	char thing4[50];
};

void vectorsandstructs()
{ 
	vector<str>shit_vector;

	//zero struct
	//for strings you will need to allocate and memset
	//UNLESS you statically set them
	str shit={0};
	shit.thing1 = 1;
	shit.thing2 = true;
	shit.thing3 = 0.01;
	strcpy(shit.thing4, "shitfuck");
	shit_vector.push_back(shit);

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
	loops();
	logic();
	vectorsandstructs();
	
}
