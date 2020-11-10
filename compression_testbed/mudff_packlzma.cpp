#include <cstdlib>
#include <vector>

using namespace std;

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

#define VER         "0.3"

int decomp(char * file, char* directory);
int comp(char * file, char* directory);
void std_err(void);
void io_err(void);



const int MOD_ADLER = 65521;

DWORD adler32(unsigned char *data, size_t len) /* where data is the location of the data in physical memory and
											   len is the length of the data in bytes */
{
	DWORD a = 1, b = 0;
	size_t index;
	/* Process each byte of the data in order */
	for (index = 0; index < len; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}
	return (b << 16) | a;
}

extern "C"
{
#include "7z/LzmaEnc.h"
}


void* Alloc(void *p, size_t size) { return malloc(size); }
void Free(void *p, void *address) { if (address) free(address); }
ISzAlloc alloc = { Alloc, Free };

extern "C"  unsigned char* __stdcall compress_lzma(unsigned char* pvInput, DWORD dwInputSize, DWORD* pdwOutputSize)
{
	unsigned char* pvOutput;
	unsigned int Packed_Size = dwInputSize * 2;
	BYTE *Packed_Mem = (BYTE *)malloc(Packed_Size);
	memset(Packed_Mem, 0, Packed_Size);
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.dictSize = 1 << 16;
	props.lc = 3;
	props.lp = 0;
	props.pb = 2;
	props.numThreads = 1;
	SizeT s = LZMA_PROPS_SIZE;
	SRes err = LzmaEncode((Byte*)Packed_Mem + LZMA_PROPS_SIZE, (SizeT*)&Packed_Size,
		(Byte*)pvInput, dwInputSize, &props, (Byte*)Packed_Mem, &s, 1, NULL, &alloc, &alloc);
	Packed_Size += LZMA_PROPS_SIZE;
	if (err != SZ_OK)
	{
		free(Packed_Mem);
		free(pvOutput);
		return NULL;
	}
	*pdwOutputSize = Packed_Size;
	pvOutput = (unsigned char*)malloc(*pdwOutputSize);
	memcpy(pvOutput, Packed_Mem, *pdwOutputSize);
	free(Packed_Mem);
	return pvOutput;

}

/* Work-memory needed for compression. Allocate memory in units
* of 'lzo_align_t' (instead of 'char') to make sure it is properly aligned.
*/
extern "C" DWORD _stdcall get_lzmadepackersize();
extern "C" DWORD _stdcall get_lzmadepackerptr();

#include "entropy/lzdatagen.h"
#include "entropy/pcg_basic.h"
#include <time.h>

#define BLOCK_SIZE (1024 * 1024 * 2)
extern "C" DWORD _stdcall get_lzmadepackersize();
extern "C" DWORD _stdcall get_lzmadepackerptr();


#ifdef _LZMA_PROB32
#define CProb UInt32
#else
#define CProb UInt16
#endif

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

typedef struct _CLzmaProperties
{
	int lc;
	int lp;
	int pb;
}CLzmaProperties;

#define LzmaGetNumProbs (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (3 + 2)))

#define kLzmaNeedInitId (-2)

typedef struct _CLzmaDecoderState
{
	CLzmaProperties Properties;
	CProb* Probs;
} CLzmaDecoderState;

__forceinline int LzmaDecodeProperties(CLzmaProperties* propsRes, const unsigned char* propsData, int size)
{
	unsigned char prop0;
	if (size < LZMA_PROPERTIES_SIZE)
		return LZMA_RESULT_DATA_ERROR;
	prop0 = propsData[0];
	if (prop0 >= (9 * 5 * 5))
		return LZMA_RESULT_DATA_ERROR;
	{
		for (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
		for (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
		propsRes->lc = prop0;
	}
	return LZMA_RESULT_OK;
}
#define LzmaGetNumProbs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))


int main(int argc, char *argv[]) {
	DWORD compressed;
	DWORD original_sz = BLOCK_SIZE;
	uint64_t seed = time(NULL);
	
	BYTE *Original = (unsigned char*)malloc(BLOCK_SIZE);
	pcg32_srandom(seed, 0xC0FFEE);
	lzdg_generate_data(Original, original_sz, 4.0, 3.0, 3.0);
	DWORD adler1 = adler32(Original, original_sz);
	printf("Original data adler is 0x%04x\n", adler1);
	BYTE* compressed_data = compress_lzma(Original, original_sz, &compressed);

	typedef int(_stdcall *tdecomp) (UInt16* workmem,
		const unsigned char *inStream, SizeT inSize,
		unsigned char *outStream, SizeT outSize);
	PVOID testmem = (unsigned char*)malloc(original_sz);
	int result;

	DWORD unpacker_sz = get_lzmadepackersize();
	DWORD unpacker_ptr = get_lzmadepackerptr();



	CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
	unsigned char properties[LZMA_PROPERTIES_SIZE];
	for (int i = 0; i < LZMA_PROPERTIES_SIZE;i++)
	properties[i] = compressed_data[i];
	LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE);
	state.Probs = (CProb*)malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));

	 typedef void(_stdcall * tdefilt)(CLzmaDecoderState* workmem,
	const unsigned char *inStream,
	unsigned char *outStream, SizeT outSize);
  tdefilt codefilt = (tdefilt)unpacker_ptr;


	codefilt(&state, compressed_data + LZMA_PROPS_SIZE, (unsigned char*)testmem, (SizeT)original_sz);
	DWORD adler2 = adler32((unsigned char*)testmem, original_sz);
	if (adler1 != adler2)printf("File not equal\n");
	else printf("File equal\n");
	free(Original);
	free(compressed_data);
	free(state.Probs);
	
	return 0;
}
