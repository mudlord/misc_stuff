#include <stdio.h>
#include "dirent.h"
#include <cstdlib>
#include <stdint.h>
#include <stdbool.h>

#pragma pack(1)
struct filehead {
	uint32_t magic;
	uint32_t numentries;
};
struct fileentry {
	int64_t uncomp_size;
	int64_t comp_size;
	int64_t offset; 
	uint32_t hash; 
};
#pragma pop

static const unsigned int tinf_crc32tab[16] = {
	0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190,
	0x6B6B51F4, 0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344,
	0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0, 0x86D3D2D4, 0xA00AE278,
	0xBDBDF21C
};

unsigned int tinf_crc32(const void* data, unsigned int length)
{
	const unsigned char* buf = (const unsigned char*)data;
	unsigned int crc = 0xFFFFFFFF;
	unsigned int i;

	if (length == 0) {
		return 0;
	}

	for (i = 0; i < length; ++i) {
		crc ^= buf[i];
		crc = tinf_crc32tab[crc & 0x0F] ^ (crc >> 4);
		crc = tinf_crc32tab[crc & 0x0F] ^ (crc >> 4);
	}

	return crc ^ 0xFFFFFFFF;
}


void readfile()
{
	FILE *fp = fopen("arch.mpk", "rb");
	struct filehead head2;
	fread(&head2, sizeof(struct filehead), 1, fp);
	struct fileentry* p2 = (struct fileentry*)malloc(sizeof(struct fileentry) * head2.numentries);
	fread(p2, sizeof(struct fileentry), head2.numentries, fp);

	int filedata_offset = sizeof(struct filehead) + sizeof(struct fileentry) * head2.numentries;

	int i = 0;
	do
	{
		printf("Hash: 0x%04X\n", (p2 + i)->hash);
		printf("Offset: 0x%04X\n", (p2 + i)->offset) + filedata_offset;
		printf("Compressed size: 0x%04X\n", (p2 + i)->comp_size);
		printf("Uncompressed size: 0x%04X\n", (p2 + i)->uncomp_size);
		printf("\n");
		i++;
	} while (i != head2.numentries);
	free(p2);
}

void main()
{
	DIR* dir;
	struct dirent* ent;
	uint64_t num_files = 0;
	struct filehead header = { 0xDEAD1988,0 };
	struct fileentry* p = NULL;
	uint64_t file_offset = 0;
	
	uint8_t* buffer = NULL;
	size_t buffersize;

	uint64_t szstruct = sizeof(struct fileentry);
	if ((dir = opendir(".")) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (!strcmp(ent->d_name, "."))
				continue;
			if (!strcmp(ent->d_name, ".."))
				continue;
			if (!strcmp(ent->d_name,"arch.mpk"))
				continue;
			if (!strcmp(ent->d_name,"archivetest.exe"))
				continue;

			header.numentries++;
			num_files++;

			printf("%s\n", ent->d_name);

			if (!p)
				p = (struct fileentry*)malloc(sizeof(struct fileentry));
			else
				p = (struct fileentry*)realloc(p, sizeof(struct fileentry) * num_files);

			FILE* filep;
			size_t filesize = 0;;
			filep = fopen(ent->d_name, "rb");
			if (filep == NULL) { fputs("File error", stderr); return; }
			fseek(filep, 0, SEEK_END);
			filesize = ftell(filep);
			rewind(filep);

			buffersize += filesize;

			uint32_t hash;
			
			if (!buffer)
			{
				buffer = (uint8_t*)malloc(sizeof(uint8_t) * buffersize);
				if (buffer == NULL) { fputs("Memory error", stderr); return; }
				size_t result = fread(buffer, 1, filesize, filep);


				hash = tinf_crc32(buffer, filesize);


				if (result != filesize) { fputs("Reading error", stderr); return; }
			}
			else
			{
				uint8_t* newbuf = (uint8_t*)realloc(buffer, sizeof(uint8_t) * buffersize);
				if (newbuf == NULL) { fputs("Memory error", stderr); return; }
				size_t result = fread(newbuf, 1, filesize, filep);
				if (result != filesize) { fputs("Reading error", stderr); return; }

				hash = tinf_crc32(newbuf, filesize);

				buffer = newbuf;
			}
				

		
				
			fclose(filep);
				
			(p+num_files-1)->uncomp_size = filesize;
			(p+num_files-1)->offset = file_offset;
			(p+num_files-1)->comp_size = filesize;
			(p+num_files-1)->hash = hash;
			

			file_offset += filesize;
		}
		closedir(dir);


		header.numentries = num_files;
		
	
		FILE* fp= fopen("arch.mpk", "wb");
		fwrite((void*)&header, sizeof(struct filehead), 1, fp);
		fwrite((void*)&p[0], sizeof(struct fileentry), header.numentries, fp);
		fclose(fp);
		
		free(buffer);
		free(p);

		readfile();
	}

	else {

	}


}
