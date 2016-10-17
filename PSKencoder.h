#ifndef PSKENCODER_H
#define PSKENCODER_H

	#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

using namespace std;

class PSKencoder
{
private:
double phase;

public:
unsigned int sampleRate;
unsigned int baud;
double carrier;

PSKencoder()
{
	
	phase = 0;
	sampleRate = 44100;
	baud = 200;
	carrier = 1000;
}

unsigned int generate(short **data) 
{
//	static int counter=0;
//	cout<<"counter: "<<++counter<<endl<<flush;
	unsigned int length = sampleRate/baud;
	*data = new short[length];
	memset(*data, 0, length*sizeof(short) );
	
	int i;
	for(i = 0; i < length; i++)
	{
		(*data)[i] += (short)(32767.0 * sin(2*M_PI*carrier * (i) / (float) sampleRate + phase));
	}

	phase += 2*M_PI*carrier  *i / (float) sampleRate;
	phase -= 2*M_PI * (int) (phase / (2*M_PI));

	return length;
}

unsigned int generate_nextbit( short **data )  
{
	phase += 3*M_PI / 2.0;
	return generate( data );
}

unsigned int generate_bit( bool sign, short **data ) 
{
	phase += sign ? M_PI : M_PI*2.0;
	
	return generate( data );
}

unsigned int generate_bits( bool sign1, bool sign2, short **data ) 
{
	phase += sign1 ? M_PI : M_PI*2.0;
	phase += sign2 ? M_PI_2 : M_PI*2.0;
	
	return generate( data );
}

unsigned int encode(char *input, unsigned int length, short **data)
{
	*data = new short[ length * (sampleRate/baud)*9 ];
//	cout<<"*data: ["<<*data<<":"<<(*data)+length * (sampleRate/baud)*9*sizeof(short)<<"]"<<endl;;
	unsigned int offset=0;
	
	unsigned int size;
	
	short *result;
	
	for (unsigned int i=0; i<length; i++)
	{
		char byte = input[i];
		
		for (char j=7; j>=0; j--)
		{
			size = generate_bit( byte & (1<<j), &result );
			
//			cout<<"data copy up to: "<<(*data)+offset+sizeof(short)*size<<", remaining: "<<((*data)+length * (sampleRate/baud)*9*sizeof(short)) - ((*data)+offset+sizeof(short)*size) <<endl;
			memcpy ( &(*data)[offset], result, sizeof(short)*size);
			offset+=size;
//			delete[] result;
//			result = 0;
		}
		size = generate_nextbit( &result );
		memcpy ( &(*data)[offset], result, sizeof(short)*size);
		offset+=size;
//		delete[] result;
//		result=0;
	}
	
		return offset;
	}

};

#endif