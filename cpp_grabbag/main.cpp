

#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>



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


extern void loops();
extern void vectorsandstructs();
extern void classes();
extern void unique_ptrs_lambda();

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
