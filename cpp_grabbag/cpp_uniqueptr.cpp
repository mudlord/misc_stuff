

#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>

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
		return double((0x8000 - (int)0x2000 / 0x8000));
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