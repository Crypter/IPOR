#ifndef BELL202ENCODER_H
#define BELL202ENCODER_H


class BELL202encoder
{
private:
bool state;

double phase;
	
public:
	
	int baud;
	
	int sampleRate;
	
	unsigned int Freq[2];
	double angularSpeed[2];
	
	BELL202encoder()
	{
		state=0;
		phase=0;
		baud=600;
		sampleRate=48000;

		Freq[0]=2200;
		Freq[1]=1200;
		
		angularSpeed[0] = 1.0/((double)sampleRate/Freq[0]);
		angularSpeed[1] = 1.0/((double)sampleRate/Freq[1]);

	}
	
	unsigned int generate(short *buff, bool value)
	{
		if ( !value ) state^=1;
		unsigned int sampleSize = sampleRate/baud;
		for (int i=0; i<sampleSize; i++)
		{
//			buff[i] = sin( 2.0*M_PI*Freq[state] * (i) / (float) sampleRate )*32767.0;
			buff[i] = sin( 2.0*M_PI* (i*angularSpeed[state] + phase) )*32767.0;
		}
		phase += (double)sampleSize*angularSpeed[state];
		phase -= (int) phase;
		
//		cout<<"DEBUG PHASE: "<<phase<<"|"<<(sampleSize-1)*angularSpeed[state]<<endl<<flush;
//		for (int i=0; i<sampleSize/10; i++)
//		{
//			buff[i] *= (double) i/(sampleSize/10.0);
//			buff[sampleSize-i-1] *= (double) i/(sampleSize/10.0);
//		}
		return sampleSize;
	}
	
	unsigned int encode(char *data, unsigned int length, short *result)
	{
		unsigned int offset=0;
		for (unsigned int i=0; i<length; i++)
		{
//			cout<<(unsigned int) byte%256<<endl;
			for (int j=7; j>=0; j--)
			{
				offset+=generate( &result[offset],  data[i]&(1<<j) );
//				cout<<"DEBUG ENCODING: "<<(bool) (data[i]&(1<<j)) <<endl<<flush;
			}
		}
		state=0;
		return offset;
	}
	
	
	
};

#endif