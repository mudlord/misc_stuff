#include <stdio.h>
#include <stdlib.h> 
#include <windows.h>

 #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#ifdef WIN32
    #include <direct.h>
    #include <malloc.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

void std_err(void);
void io_err(void);

typedef struct{
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
}PALCOL;

#pragma pack(push, 1) // exact fit - no padding
struct head {
	DWORD  signature; 
	DWORD   width;
	DWORD   height;
	PALCOL pallete[256];
};
#pragma pack(pop)

static void quantize(unsigned char *rgba, int rgbaSize, int sample, unsigned char *map, int numColors) {
	// defs for freq and bias
	const int intbiasshift = 16; /* bias for fractions */
	const int intbias = (((int) 1) << intbiasshift);
	const int gammashift = 10; /* gamma = 1024 */
	const int betashift = 10;
	const int beta = (intbias >> betashift); /* beta = 1/1024 */
	const int betagamma = (intbias << (gammashift - betashift));

	// defs for decreasing radius factor
	const int radiusbiasshift = 6; /* at 32.0 biased by 6 bits */
	const int radiusbias = (((int) 1) << radiusbiasshift);
	const int radiusdec = 30; /* factor of 1/30 each cycle */

	// defs for decreasing alpha factor
	const int alphabiasshift = 10; /* alpha starts at 1.0 */
	const int initalpha = (((int) 1) << alphabiasshift);

	// radbias and alpharadbias used for radpower calculation
	const int radbiasshift = 8;
	const int radbias = (((int) 1) << radbiasshift);
	const int alpharadbshift = (alphabiasshift + radbiasshift);
	const int alpharadbias = (((int) 1) << alpharadbshift);

	sample = sample < 1 ? 1 : sample > 30 ? 30 : sample;
	int network[256][3];
	int bias[256] = {}, freq[256];
	for(int i = 0; i < numColors; ++i) {
		// Put nurons evenly through the luminance spectrum.
		network[i][0] = network[i][1] = network[i][2] = (i << 12) / numColors;
		freq[i] = intbias / numColors; 
	}
	// Learn
	{
		const int primes[5] = {499, 491, 487, 503};
		int step = 4;
		for(int i = 0; i < 4; ++i) {
			if(rgbaSize > primes[i] * 4 && (rgbaSize % primes[i])) { // TODO/Error? primes[i]*4?
				step = primes[i] * 4;
			}
		}
		sample = step == 4 ? 1 : sample;

		int alphadec = 30 + ((sample - 1) / 3);
		int samplepixels = rgbaSize / (4 * sample);
		int delta = samplepixels / 100;
		int alpha = initalpha;
		delta = delta == 0 ? 1 : delta;

		int radius = (numColors >> 3) * radiusbias;
		int rad = radius >> radiusbiasshift;
		rad = rad <= 1 ? 0 : rad;
		int radSq = rad*rad;
		int radpower[32];
		for (int i = 0; i < rad; i++) {
			radpower[i] = alpha * (((radSq - i * i) * radbias) / radSq);
		}

		// Randomly walk through the pixels and relax neurons to the "optimal" target.
		for(int i = 0, pix = 0; i < samplepixels;) {
			int r = rgba[pix + 0] << 4;
			int g = rgba[pix + 1] << 4;
			int b = rgba[pix + 2] << 4;
			int j = -1;
			{
				// finds closest neuron (min dist) and updates freq 
				// finds best neuron (min dist-bias) and returns position 
				// for frequently chosen neurons, freq[k] is high and bias[k] is negative 
				// bias[k] = gamma*((1/numColors)-freq[k]) 

				int bestd = 0x7FFFFFFF, bestbiasd = 0x7FFFFFFF, bestpos = -1;
				for (int k = 0; k < numColors; k++) {
					int *n = network[k];
					int dist = abs(n[0] - r) + abs(n[1] - g) + abs(n[2] - b);
					if (dist < bestd) {
						bestd = dist;
						bestpos = k;
					}
					int biasdist = dist - ((bias[k]) >> (intbiasshift - 4));
					if (biasdist < bestbiasd) {
						bestbiasd = biasdist;
						j = k;
					}
					int betafreq = freq[k] >> betashift;
					freq[k] -= betafreq;
					bias[k] += betafreq << gammashift;
				}
				freq[bestpos] += beta;
				bias[bestpos] -= betagamma;
			}

			// Move neuron j towards biased (b,g,r) by factor alpha
			network[j][0] -= (network[j][0] - r) * alpha / initalpha;
			network[j][1] -= (network[j][1] - g) * alpha / initalpha;
			network[j][2] -= (network[j][2] - b) * alpha / initalpha;
			if (rad != 0) {
				// Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
				int lo = j - rad;
				lo = lo < -1 ? -1 : lo;
				int hi = j + rad;
				hi = hi > numColors ? numColors : hi;
				for(int jj = j+1, m=1; jj < hi; ++jj) {
					int a = radpower[m++];
					network[jj][0] -= (network[jj][0] - r) * a / alpharadbias;
					network[jj][1] -= (network[jj][1] - g) * a / alpharadbias;
					network[jj][2] -= (network[jj][2] - b) * a / alpharadbias;
				}
				for(int k = j-1, m=1; k > lo; --k) {
					int a = radpower[m++];
					network[k][0] -= (network[k][0] - r) * a / alpharadbias;
					network[k][1] -= (network[k][1] - g) * a / alpharadbias;
					network[k][2] -= (network[k][2] - b) * a / alpharadbias;
				}
			}

			pix += step;
			pix = pix >= rgbaSize ? pix - rgbaSize : pix;

			// every 1% of the image, move less over the following iterations.
			if(++i % delta == 0) {
				alpha -= alpha / alphadec;
				radius -= radius / radiusdec;
				rad = radius >> radiusbiasshift;
				rad = rad <= 1 ? 0 : rad;
				radSq = rad*rad;
				for (j = 0; j < rad; j++) {
					radpower[j] = alpha * ((radSq - j * j) * radbias / radSq);
				}
			}
		}
	}
	// Unbias network to give byte values 0..255
	for (int i = 0; i < numColors; i++) {
		map[i*3+0] = network[i][0] >>= 4;
		map[i*3+1] = network[i][1] >>= 4;
		map[i*3+2] = network[i][2] >>= 4;
	}
}

void load_file(char* file)
{
	PALCOL *palette;
	head header_struct = {0};
	FILE* fd = fopen(file, "rb");
	fread(&header_struct,sizeof(header_struct), 1, fd);
	int pos = ftell(fd);
	int width = header_struct.width;
	int height = header_struct.height;
	palette = header_struct.pallete;

	BYTE * colortable = new BYTE[width * height];
	ZeroMemory(colortable,sizeof(colortable));
	fread(colortable,width*height, 1, fd);
	fclose(fd);

	BYTE *output = new BYTE[width * height * 4];
	for(int i=0,k=0; i<width * height ;i++)
	{
		int palleteentry = colortable[i];
		output[k++]=palette[palleteentry].red;
		output[k++]=palette[palleteentry].green;
		output[k++]=palette[palleteentry].blue;
		output[k++]=palette[palleteentry].alpha;

	} 
	stbi_write_png("test.png", width, height, 4, output, width*4);

}

void save_file(char* image, char* comp_file)
{
	unsigned char localPalTbl[0x300] = {0};
	unsigned char *pallete_ = localPalTbl;
	int width,height,comp;
	unsigned char* image_data =  stbi_load(image, &width, &height, &comp, 4);
	quantize(image_data, width*height*4, 1, (unsigned char*)pallete_, 255);
	int size = width*height;

	PALCOL pal[256] = {};
	for(int i = 0; i < 255; ++i) {
		pal[i].red= pallete_[i*3+0];
		pal[i].green = pallete_[i*3+1];
		pal[i].blue = pallete_[i*3+2];
		pal[i].alpha = 255;
	}

	unsigned char *palletized = (unsigned char *)malloc(size);
	{
		for(int k = 0; k < size*4; k+=4) {
			int rgba[4] = { image_data[k+0], image_data[k+1], image_data[k+2],image_data[k+3] };
			int bestd = 0x7FFFFFFF, best = -1;
			// TODO: exhaustive search. do something better.
			for(int i = 0; i < 255; ++i) {
				int rr = pallete_[i*3+0]-rgba[0];
				int gg = pallete_[i*3+1]-rgba[1];
				int bb = pallete_[i*3+2]-rgba[2];
				int d = bb*bb + gg*gg + rr*rr;
				if(d < bestd) {
					bestd = d;
					best = i;
					
				}
			}
			pal[best].alpha = rgba[3];
			palletized[k/4] = best;
		}
	}
	free(image_data);
	head header_struct = {0};
	FILE* fd = fopen(comp_file, "wb");
	header_struct.signature = 0x1211988;
	header_struct.width = width;
	header_struct.height = height;
	memcpy(header_struct.pallete,pal,256*sizeof(PALCOL));
	fwrite(&header_struct, sizeof(header_struct), 1, fd);
	fwrite(palletized,size, 1, fd);
	fclose(fd);
	printf("File made successfully.\n");

	load_file(comp_file);
}


void std_err(void) {
    perror("\nError");
    exit(1);
}

void io_err(void) {
    fputs("\nError: I/O error, the file is incomplete\n", stdout);
    exit(1);
}

#define VER         "0.2"

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    fputs("\n"
    "MIFMAKER "VER":\n"
	"a simple/easy image format for demos/intros\n"
    "by mudlord\n"
    "web:    mudlord.info\n"
    "\n", stdout);

	save_file("original.png", "out.mif");

	printf("\n");
	system("pause");
	return 0;
}
