#ifndef FSKENCODER_H
#define FSKENCODER_H


class FSKencoder
{
public:
	
	int baud;
	
	int sampleRate;
	
	unsigned int Freq[6];
	
	FSKencoder()
	{
		baud=300;
		sampleRate=48000;
//		Freq[0]=441;
//		Freq[1]=882;
//		Freq[2]=1323;
//		Freq[3]=1764;
//		Freq[4]=2646;

//		Freq[0]=1000;
//		Freq[1]=1500;
//		Freq[2]=2000;
//		Freq[3]=2500;
//		Freq[4]=3000;
//		Freq[5]=500;

		Freq[0]=1200;
		Freq[1]=1800;
		Freq[2]=2400;
		Freq[3]=3000;
		Freq[4]=3600;
		Freq[5]=600;
		
//		Freq[0]=1800;
//		Freq[1]=1800;
//		Freq[2]=1800;
//		Freq[3]=1800;
//		Freq[4]=1800;
//		Freq[5]=1800;

	}
	
	unsigned int generate(short *buff, char value)
	{
		unsigned int sampleSize = sampleRate/baud;
		for (int i=0; i<sampleSize; i++)
		{
			buff[i] = sin( 2.0*M_PI*Freq[value] * (i) / (float) sampleRate )*32767.0;
		}
		
		for (int i=0; i<sampleSize/10; i++)
		{
			buff[i] *= (double) i/(sampleSize/10.0);
			buff[sampleSize-i-1] *= (double) i/(sampleSize/10.0);
		}
		return sampleSize;
	}
	
	unsigned int generateBits(short *buff, bool first, bool second)
	{
		char value;
		value=first<<1 | second;
		return generate(buff, value);
	}
	
	unsigned int encode(char *data, unsigned int length, short *result)
	{
		unsigned int offset=0;
		for (unsigned int i=0; i<length; i++)
		{
			offset+=generate(&result[offset], 5);
			char byte=data[i];
//			cout<<(unsigned int) byte%256<<endl;
			for (int j=7; j>0; j-=2)
			{
				offset+=generateBits(&result[offset],  byte&(1<<j), byte&(1<<j-1));
			}
			offset+=generate(&result[offset], 4);
		}
		return offset;
	}
	
	
	
};

#endif