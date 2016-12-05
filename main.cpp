#include <iostream>
#include <alsa/asoundlib.h>
#include <math.h>
#include <thread>
#include <mutex>


#include "FSKencoder.h"
#include "FSKdecoder.h"
#include "ALSAplayback.h"
#include "ALSArecorder.h"
#include "TUNio.h"
#include "Hamming.h"
#include "fourier.h"
#include "BELL202decoder.h"
#include "BELL202encoder.h"


using namespace std;


template <class T> class protectedVar
{
	std::mutex lockout;
public:
	T content; 
	void lock() { lockout.lock(); }
	void unlock() { lockout.unlock(); }
	
};


void backgroundDecoder()
{
	cout<<"DECODING..."<<endl<<flush;
	unsigned int sampleSize = 512;
	short buffer[sampleSize];

//	Hamming fec;

	ALSArecorder record;
	record.init();
	BELL202decoder decoder;
	decoder.run();
	
	for (;;)
	{
		record.record(buffer, sampleSize);
//		cout<<"decoding loop\n"<<flush;
		decoder.addToQueue(buffer, sampleSize);
		
		bool tunedIn=0;
		for(int i=0; decoder.resultingData.size()>8 && i<256; i++)
		{
//			cout<<(unsigned int) decoder.resultingData.front()%256<<flush;
			if (!tunedIn)
			if (decoder.resultingData[0]==0 && decoder.resultingData[1]==1 && decoder.resultingData[2]==1 && decoder.resultingData[3]==1 && decoder.resultingData[4]==1 && decoder.resultingData[5]==1 && decoder.resultingData[6]==1 && decoder.resultingData[7]==0)
			{
				cout<<"start"<<endl<<flush;
				tunedIn=1;
				for (int i=0; i<8; i++) decoder.resultingData.pop_front();
			}
			else
			{
				decoder.resultingData.pop_front();
				continue;
			}
			
			for(int j=0; decoder.resultingData.size()>8 && j<256; j++)
			{
				char byte=0;
				for (int i=0; i<8; i++)
				{
					byte = (byte<<1) | decoder.resultingData.front();
					decoder.resultingData.pop_front();
				}
			if ( byte == 126) {tunedIn=0; cout<<"end"<<endl<<flush; break;}
//				cout<<byte<<" "<<(unsigned int )byte%256<<endl<<flush;
			if (tunedIn) cout<<byte<<flush;
			}
			
		}
	}
}

int main(int argc, char* argv[]) 
{


srand(time(0));

unsigned int length;
char netBuffer[500];
short buffer[1900000];

//thread decodingThread(backgroundDecoder);

//TUNio networkDevice;
//networkDevice.init();

BELL202encoder encoder;

ALSAplayback playback;
playback.init();


BELL202decoder debug;
debug.run();

cout<<"ENCODING..."<<endl;
unsigned int counter=0;

for(;;)
	{
		sprintf(netBuffer, "\xFF\xFF\xFF\xFF\xFF\x7E[%d]Text written for testing purposes, length of a lot of characters\n\x7E\xFF", counter++);
		length = strlen(netBuffer);
//		netBuffer[0] = 126;
//		length = 8;
		
		
//		length = networkDevice.receive(netBuffer, 1500);
		
		length = encoder.encode(netBuffer, length, buffer);
		
//		double dbuff[length];
//		for (int i=0; i<length; i++) cout<<buffer[i]<<endl<<flush;
//		
//		highpass(dbuff, length, 6000, 48000);
//		
//		for (int i=0; i<length; i++) buffer[i] = dbuff[i] * 32727.0 ;
		
		
debug.addToQueue(buffer, length);
		
		
		
		
		
		
				bool tunedIn=0;
		for(int i=0; debug.resultingData.size()>8 && i<256; i++)
		{
//			cout<<(unsigned int) decoder.resultingData.front()%256<<flush;
			if (!tunedIn)
			if (debug.resultingData[0]==0 && debug.resultingData[1]==1 && debug.resultingData[2]==1 && debug.resultingData[3]==1 && debug.resultingData[4]==1 && debug.resultingData[5]==1 && debug.resultingData[6]==1 && debug.resultingData[7]==0)
			{
				cout<<"start"<<endl<<flush;
				tunedIn=1;
				for (int i=0; i<8; i++) debug.resultingData.pop_front();
			}
			else
			{
				debug.resultingData.pop_front();
				continue;
			}
			
			for(int j=0; debug.resultingData.size()>8 && j<256; j++)
			{
				char byte=0;
				for (int i=0; i<8; i++)
				{
					byte = (byte<<1) | debug.resultingData.front();
					debug.resultingData.pop_front();
				}
			if ( byte == 126) {tunedIn=0; cout<<"end"<<endl<<flush; break;}
//				cout<<byte<<" "<<(unsigned int )byte%256<<endl<<flush;
			if (tunedIn) cout<<byte<<flush;
			}
			
		}
		
		
		
		
//		playback.play(buffer, length);

//		unsigned int len = debug.resultingData.size();
//		
//		if ( len>2 )
//		{
//			Hamming fec;
//			
//			char *tempData = new char[len];
//			
//			for (unsigned int i=0; i<len/2; i++)
//			{
//				tempData[i*2]=debug.resultingData.front();
//				debug.resultingData.pop();
//				
//				tempData[i*2+1]=debug.resultingData.front();
//				debug.resultingData.pop();
//			}
//			
//			tempData=fec.decode(tempData, len);
//			tempData[len/2]=0;
//			cout<<tempData<<flush;
//		}
//		cout<<"\ncounter: "<<counter<<endl<<flush;
		sleep(2);
	}


playback.stop();
//decodingThread.join();
	return 0;
	
}



/*
 
 */



//short *newBuf = new short[length+500];
//for (int i=0; i<500; i++) newBuf[i]=(rand()%1000)-500;
//
//memcpy(&newBuf[500], buffer, length*sizeof(short));
//
//FSKdecoder debug;
//debug.decode();
//debug.addToQueue(newBuf, length+500);
//debug.addToQueue(newBuf, length+500);
//debug.addToQueue(newBuf, length+500);
//
//debug.run();
//
