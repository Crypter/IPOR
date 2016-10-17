#ifndef HAMMING_H
#define HAMMING_H

#include <bitset>

using namespace std;

class Hamming
{
public:
char * encode(char *data, unsigned int length)
{
	unsigned char const codebook[] = {0x00, 0xD2, 0x55, 0x87, 0x99, 0x4B, 0xCC, 0x1E, 0xE1, 0x33, 0xB4, 0x66, 0x78, 0xAA, 0x2D, 0xFF};

	char *result = new char[length*2];
	for (unsigned int i=0; i<length; i++)
	{
		result[i*2] = codebook[ data[i]>>4 ];
		result[i*2+1] = codebook[ data[i]&0x0F ];
		
//		std::bitset<8> y1(result[i*2]);
//		std::bitset<8> y2(result[i*2+1]);
//		std::bitset<8> x(data[i]);
//		cout<<x<<"="<<y1<<" "<<y2<<endl;
	}
	return result;
}

char * decode(char *data, unsigned int length)
{
	unsigned char const codebook[] = {0xAA, 0x66, 0x1E, 0xFF};
	unsigned char const checkbook[] = {0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF, 0x1 };

	char *result = new char[length/2];
	
	for (unsigned int i=0; i<length; i++)
	{
		unsigned char test=0;
		bitset<8> temp(data[i]&codebook[0]);
		test = temp.count()%2;
		
		temp = data[i]&codebook[1];
		test = (test<<1) | (temp.count()%2);
		
		temp = data[i]&codebook[2];
		test = (test<<1) | (temp.count()%2);
		
		temp = data[i]&codebook[3];
		test = (test<<1) | (temp.count()%2);
		
		
		if ( !(i%2) ) result[i/2]=(data[i]&32)<<2 | (data[i]&8)<<3 | (data[i]&4)<<3 | (data[i]&2)<<3;
		else result[i/2] = result[i/2] | (data[i]&32)>>2 | (data[i]&8)>>1 | (data[i]&4)>>1 | (data[i]&2)>>1;
		
		if ( test ) //no error
		{
			for(unsigned int j=0; j<8; j++)
			{
				if ( test==checkbook[j] )
				{
					data[i] ^= (1<<(7-j) );
					break;
				}
			}
		}
		
		if ( !(i%2) ) result[i/2]=(data[i]&32)<<2 | (data[i]&8)<<3 | (data[i]&4)<<3 | (data[i]&2)<<3;
		else result[i/2] = result[i/2] | (data[i]&32)>>2 | (data[i]&8)>>1 | (data[i]&4)>>1 | (data[i]&2)>>1;
		
	}
	
	return result;
}

};

#endif