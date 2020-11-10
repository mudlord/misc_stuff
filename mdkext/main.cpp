#include <cstdlib>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef WIN32
    #include <direct.h>
    #include <malloc.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

#define VER         "0.2"

int decomp(char * file, char* directory);
int comp(char * file, char* directory);
void std_err(void);
void io_err(void);

struct head {
	DWORD  magic; //file magic?
	unsigned char  fname[12]; //archive
	DWORD  magic_sub8; 
	DWORD  fileznum;    //number of files
} head;
struct filez{
    unsigned char  fname[16]; //filename
	DWORD  fileoff;       // file offset
    DWORD  filesize;       // filesize
};

int main(int argc, char *argv[]) {
  
    setbuf(stdout, NULL);
    fputs("\n"
    "MDK SNI archiver "VER"\n"
    "by mudlord\n"
    "\n", stdout);

	if(argc < 3) {
		printf("\nUsage: %s -c <file> <source_directory>\n"
			   "Otherwise it unpacks.\n",
		argv[0]);
		exit(1);
	}

	if (strcmp("-c", argv[1]) == 0)
	{
	comp(argv[2],argv[3]);
	}
	else
	{
	decomp(argv[1],argv[2]);
	}
		
	return(0);
   
}

BYTE *Load_Input_File(char *FileName, DWORD *Size)
{
	BYTE *Memory;
	FILE *Input = fopen(FileName, "rb");
	if(!Input) return(NULL);
	// Get the filesize
	fseek(Input, 0, SEEK_END);
	*Size = ftell(Input);
	fseek(Input, 0, SEEK_SET);
	Memory = (BYTE *) malloc(*Size);
	if(!Memory) return(NULL);
	if(fread(Memory, 1, *Size, Input) != (size_t) *Size) return(NULL);
	if(Input) fclose(Input);
	Input = NULL;
	return(Memory);
}

int decomp (char * file, char * directory)
{
	std::vector<filez> entries;
	int             err, num_files;
	FILE *fd = fopen(file, "rb");
	if(!fd) std_err();
	err = fread(&head, sizeof(head), 1, fd);
	if(err != 1) io_err();
	printf("\n"
	"Archive filename: %s\n"
	"Total files:     %u\n"
	"Magic (DWORD 1): %08X\n"
	"Magic (DWORD 2): %08X\n\n",
	head.fname,head.fileznum,head.magic,head.magic_sub8);

	num_files = head.fileznum;
	fseek(fd, sizeof(head), SEEK_SET);
	for (int i = 0; i < head.fileznum; i++)
	{
		filez entry;
		fread(&entry, sizeof(filez), 1, fd);
		entries.push_back(entry);
	}
	err = chdir( directory);
	if(err < 0) std_err();
	for (int j=0;j<num_files;j++)
	{ 
		filez &entry = entries[j];
		BYTE * data = (BYTE*)malloc(entry.filesize);
		memset(data,0,entry.filesize);
		rewind(fd);
		fseek(fd, entry.fileoff, SEEK_SET);
		fread(data,entry.filesize,1,fd);
		printf("Unpacked File: %s (%d bytes at %08X)...\n", entry.fname, entry.filesize,  entry.fileoff);
		FILE *fdout = fopen((const char*)entry.fname, "wb");
		fwrite(data,entry.filesize,1,fdout);
		free(data);
		fclose(fdout);
	}
	printf("Files unpacked successfully!\n");
	fclose(fd);
}

int comp(char * file, char* directory)
{
	FILE *fd,*fdout;
	int  err;
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	static int      winnt = -1;
	OSVERSIONINFO   osver;
	DWORD dwError=0;
	int num_files = 0;
	std::vector<filez> entries;
	filez file_struct;

	strcpy(szDir, directory);
	strcat(szDir,"\\*.*");

	hFind = FindFirstFile(szDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		printf("Can't find files in directory!\n");
		return dwError;
	} 

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		}
		else
		{
			filez entry;
			memset(entry.fname,0,16);
			entry.fileoff = 0xB00BFACE;
			entry.filesize = 0xB00BFACE;
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			memcpy(entry.fname,ffd.cFileName,16);
			entries.push_back(entry);
			num_files++;
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	//write archive header
	fd = fopen(file, "wb");
	if(!fd) std_err();
	head.fileznum = num_files;
	head.magic = 0x12111988;
	head.magic_sub8 = 0x12111988 - 8;
	memset(head.fname,0,12);
	memcpy(head.fname,"BOOBIEZ.SND",strlen("BOOBIEZ.SND"));
	err = fwrite(&head, sizeof(head), 1, fd);
	if(err != 1) io_err();

	//prepping file headers
	for (int i = 0; i< num_files; i++)
	{
		memset(file_struct.fname,0,16);
		file_struct.filesize = 0xB00BFACE;
		file_struct.fileoff = 0xFEEDFACE;
		err = fwrite(&file_struct,sizeof(filez), 1, fd);
	}

	err = chdir(directory);
	if(err < 0) std_err();

	for (int j=0;j<num_files;j++)
	{
		filez &entry = entries[j];
		BYTE *Input_Mem;
		DWORD Input_Size;
		unsigned int Packed_Size;
		DWORD cur_pos, file_pos;
		cur_pos = ftell(fd);
		Input_Mem = Load_Input_File((char*)entry.fname, &Input_Size);
		Packed_Size = Input_Size;
		printf("Packed %s (%d of %d bytes at %08X)...\n", entry.fname, Packed_Size,Input_Size, cur_pos);
		fwrite(Input_Mem,Packed_Size,1,fd);
		file_pos = ftell(fd);
		free(Input_Mem);
		//fix the file headers
		rewind(fd);
		fseek(fd, sizeof(head) + j * sizeof(filez), SEEK_SET);
		memset(file_struct.fname,0,16);
		memcpy(file_struct.fname,entry.fname,strlen((const char*)entry.fname));
		file_struct.filesize = Packed_Size;
		file_struct.fileoff = cur_pos;
		fwrite(&file_struct,sizeof(filez),1,fd);
		fseek(fd,file_pos,SEEK_SET);
	}

	printf("Files packed successfully!\n");
	fclose(fd);
	return 0;
}


void std_err(void) {
    perror("\nError");
    exit(1);
}

void io_err(void) {
    fputs("\nError: I/O error, the file is incomplete or the disk space is finished\n", stdout);
    exit(1);
}