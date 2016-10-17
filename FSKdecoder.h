#ifndef FSKDECODER_H
#define FSKDECODER_H

#include "fourier.h"
#include <queue>
#include <mutex>
#include <thread>

using namespace std;


	
class FSKdecoder
{
private:
struct bufferPiece
	{
		short *content;
		unsigned int length;
	};

public:
	
	int baud;
	
	int sampleRate;
	
	unsigned int Freq[6];
	
	queue<bufferPiece> buffers;
	queue<char> resultingData;
	
	mutex bufferSync;
	thread decodingThread;
	
	
	FSKdecoder()
	{
		baud=300;
		sampleRate=48000;
		
		Freq[0]=1200;
		Freq[1]=1800;
		Freq[2]=2400;
		Freq[3]=3000;
		Freq[4]=3600;
		Freq[5]=600;

//		Freq[0]=800;
//		Freq[1]=800;
//		Freq[2]=1600;
//		Freq[3]=1600;
//		Freq[4]=1600;
//		Freq[5]=1600;


		
	}
	
	
	void run()
	{
		decodingThread = thread(&FSKdecoder::decode, this);
	}
	
void decode()
{
	unsigned int sampleSize = sampleRate / baud;

/* Raw data buffer */
	short* pieceBuffer = new short [sampleSize];

/* Massaged data buffer */
	double* dbuffer = new double [sampleSize];
	
	short *bigBuffer = new short[sampleSize*200];
	
	char bitData = 0;

	unsigned int offset=0;
	unsigned int bigBufferSize=0;
	
	bool tunedIn=0;
	for(;;)
	{
		//check if there's enough buffer
		while ( buffers.size() < 2 ) { usleep(500000); } 
		
		//shift everything to the beginning, make space for new round
		if (offset){
			memmove(bigBuffer, bigBuffer+offset, (bigBufferSize-offset)*sizeof(short));
			bigBufferSize-=offset;
			offset=0;
		}
		
		if (bigBufferSize<=sampleSize)
		{
			struct bufferPiece tempBuf;
			bufferSync.lock();
			tempBuf = buffers.front();
			buffers.pop();
			bufferSync.unlock();
			
			memcpy( bigBuffer+bigBufferSize, tempBuf.content, tempBuf.length*sizeof(short) );
			bigBufferSize+=tempBuf.length;
//			free(tempBuf.content);
		}

		
		//add piece of that piece to the processing buffer and tune it.
		unsigned int tuneOffset=0;
		double oldResult=-1,olderResult=-1, newResult=-1;
		if ( !tunedIn )
		do
		{
//			cout<<"Tuning offset: "<<tuneOffset<<endl<<flush;
			tunedIn=1;
			if (offset+tuneOffset+sampleSize>=bigBufferSize )
			{
				tunedIn=0;
				offset=tuneOffset;
				break;
			}
			memcpy(pieceBuffer, bigBuffer+offset+tuneOffset, sampleSize*sizeof(short) );
			for (int i = 0; i < sampleSize; i++) dbuffer[i] = pieceBuffer[i] / 32768.0; //normalize
			
			tuneOffset++;
			
			olderResult=oldResult;
			oldResult=newResult;
			
			double maximum=-1;
			for (int i=0; i<6; i++) maximum = max(maximum, goertzel_mag(sampleSize, Freq[i], sampleRate, dbuffer));
			newResult = maximum;
//			if (maximum>0.001) cout<<"Current max: "<<maximum<<endl;
		}
		while ( (newResult > oldResult || newResult > olderResult) || newResult < 0.01);
		
		if ( !tunedIn ) continue;
		
		if (oldResult!=-1) tuneOffset--;
//		cout<<"Fix at: ["<<tuneOffset<<"] "<<oldResult<<endl<<flush;

		memmove(&bigBuffer[offset], &bigBuffer[offset+tuneOffset], sizeof(short)*(bigBufferSize-tuneOffset) );
		bigBufferSize-=tuneOffset;
		
		while ( offset+sampleSize<=bigBufferSize )
		{
			memcpy(pieceBuffer, &bigBuffer[offset], sampleSize*sizeof(short) );
			offset+=sampleSize;
			
//			cout<<"Debug: "<<endl;
//				for (int i=0; i<sampleSize; i++) cout<<pieceBuffer[i]<<"->"<<dbuffer[i]<<endl;
			
			for (int i = 0; i < sampleSize; i++) dbuffer[i] = pieceBuffer[i] / 32768.0; //normalize
			
			double max=0, temp=0;
			char maxID=0;
			
			max = goertzel_mag(sampleSize, Freq[0], sampleRate, dbuffer);
			for (int i=1; i<6; i++)
			{
				temp = goertzel_mag(sampleSize, Freq[i], sampleRate, dbuffer);
				if ( temp>max ) { max=temp; maxID=i; }
			}
			
			if (max<0.01)
			{
				tunedIn=0;
//				cout<<"Failed because: "<<endl;
//				for (int i=0; i<sampleSize; i++) cout<<pieceBuffer[i]<<"->"<<dbuffer[i]<<endl;
				break;
			}
			
			if (maxID==0) bitData=(bitData<<2) | 0;
			else if (maxID==1) bitData=(bitData<<2) | 1;
			else if (maxID==2) bitData=(bitData<<2) | 2;
			else if (maxID==3) bitData=(bitData<<2) | 3;
//			else if (maxID==4) {cout<<(unsigned int) bitData%256<<" "<<flush; resultingData.push(bitData);}
			else if (maxID==4) { resultingData.push(bitData); bitData=0; }
//			else if (maxID==4) cout<<bitData<<flush;
			else if (maxID==5) bitData=0;

		}
//		cout<<endl;
//		for (int i=0; i<6; i++) cout<<"["<<Freq[i]<<"]"<<goertzel_mag(sampleSize, Freq[i], 48000, dbuffer)<<endl;
		
	}
	
	
	
}
	
	
	void addToQueue(short *buffer, int length)
	{
		bufferSync.lock();

		struct bufferPiece newPiece;
		newPiece.content=new short[length];
		memcpy(newPiece.content, buffer, length*sizeof(short));
		newPiece.length = length;

		buffers.push(newPiece);
		bufferSync.unlock();
	}
	
};

#endif
