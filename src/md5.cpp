#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned long uint64_t;

static uint32_t s[] = {
	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,	
};

static uint32_t K[] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

uint8_t* md5(char* str, int len)
{
	int nchunks = len/64+1 + (len%64 >= 56);
	char* buf = (char*) malloc(nchunks * 64);
	memcpy(buf, str, len);
	
	char* ptr = &buf[len];
	*ptr++ = 0x80;
	while((ptr-buf) % 64 != 56)
	{
		*ptr++ = 0;
	}
	uint64_t* p2 = (uint64_t*)ptr;
	*p2 = len * 8;

	uint32_t a0 = 0x67452301;
	uint32_t b0 = 0xefcdab89;
	uint32_t c0 = 0x98badcfe;
	uint32_t d0 = 0x10325476;

	for(int n = 0; n < nchunks; n++)
	{
		uint32_t* M = (uint32_t*) &buf[n*64];
		uint32_t A = a0;
		uint32_t B = b0;
		uint32_t C = c0;
		uint32_t D = d0;
	
		for(int i = 0; i < 64; i++)
		{
			uint32_t F, g;
			if (i < 16)
			{
				F = (B & C) | ((~B) & D);
				g = i;
			}
			else if (i < 32)
			{
				F = (D & B) | ((~D) & C);
				g = (i*5 + 1) % 16;
			}
			else if (i < 48)
			{
				F = B ^ C ^ D;
				g = (i*3 + 5) % 16;
			}
			else {
				F = C ^ (B | (~D));
				g = i*7 % 16;
			}

			F = F + A + K[i] + M[g];
			A = D;
			D = C;
			C = B;
			#define ROL(n, d) (((n) << (d)) | ((n) >> (32-(d))))
			B = B + ROL(F, s[i]);
			//#undef ROL
		}

		a0 += A;
		b0 += B;
		c0 += C;
		d0 += D;
	}

	free(buf);
	static uint32_t result[4];
	result[0] = a0;
	result[1] = b0;
	result[2] = c0;
	result[3] = d0;
	return &result[0];
}
;
int main(int argc, char** argv)
{
	for(int i = 1; i < argc; i++)
	{
		uint8_t* hash = 
			md5(argv[i], strlen(argv[i]));
		for(int j = 0; j < 16; j++)
		{
			printf("%02x", hash[j]);
		}
		printf("\n");
	}
	return 0;
}
