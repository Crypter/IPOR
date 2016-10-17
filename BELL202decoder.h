#ifndef BELL202DECODER_H
#define BELL202DECODER_H

#include "fourier.h"
#include <queue>
#include <deque>
#include <mutex>
#include <thread>

using namespace std;


	
class BELL202decoder
{
private:
struct bufferPiece
	{
		short *content;
		unsigned int length;
	};
bool lastState;

double absolute(double value)
{
	return value>0 ? value : value*(-1);
}

public:
	
	int baud;
	
	int sampleRate;
	
	unsigned int Freq[2];
	
	queue<bufferPiece> buffers;
	deque<char> resultingData;
	
	mutex bufferSync;
	thread decodingThread;
	
	
	BELL202decoder()
	{
		lastState=0;
		baud=600;
		sampleRate=48000;
		
		Freq[0]=2200;
		Freq[1]=1200;
		
	}
	
	
	void run()
	{
		decodingThread = thread(&BELL202decoder::decode, this);
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
	char counter=0;

	unsigned int offset=0;
	unsigned int bigBufferSize=0;
	
	bool tunedIn=0;
	for(;;)
	{
		//check if there's enough buffer
		while ( buffers.size() < 2 ) { usleep(500000); } 
		bigBuffer[offset]=0;
		//shift everything to the beginning, make space for new round
		if (offset>sampleSize)
		{
			memmove(bigBuffer, bigBuffer+offset, (bigBufferSize-offset)*sizeof(short));
			bigBufferSize-=offset;
			offset=0;
		}
		
		while (bigBufferSize<=sampleSize)
		{
			struct bufferPiece tempBuf;
			bufferSync.lock();
			tempBuf = buffers.front();
			buffers.pop();
			bufferSync.unlock();
			
			memcpy( bigBuffer+bigBufferSize, tempBuf.content, tempBuf.length*sizeof(short) );
			bigBufferSize+=tempBuf.length;
			free(tempBuf.content);
		}
		
//		unsigned int peaking=0;
//		for (unsigned int i=0; i<bigBufferSize; i++)
//		{
//			if (bigBuffer[i] == 32767) peaking++;
//			else if (peaking>=4) {cout<<"Peaking!!!"<<endl<<flush; break;}
//			else peaking=0;
//		}
		
		//add piece of that piece to the processing buffer and tune it.
		unsigned int tuneOffset=0;
		double oldResult=-1,olderResult=-1, newResult=-1;
		if ( !tunedIn )
		do
		{
//			cout<<"Tuning offset: "<<tuneOffset<<endl<<flush;
			tunedIn=1;
			if (tuneOffset+sampleSize>bigBufferSize )
			{
				tunedIn=0;
				offset=tuneOffset;
				break;
			}
			memcpy(pieceBuffer, bigBuffer+tuneOffset, sampleSize*sizeof(short) );
			for (int i = 0; i < sampleSize; i++) dbuffer[i] = pieceBuffer[i] / 32768.0; //normalize
			
			tuneOffset++;
			
			olderResult=oldResult;
			oldResult=newResult;
			
			newResult = max(goertzel_mag(sampleSize, Freq[0], sampleRate, dbuffer), goertzel_mag(sampleSize, Freq[1], sampleRate, dbuffer));
			
//			if (newResult>0.001) cout<<"Current max: "<<newResult<<" at "<<tuneOffset<<endl;
		}
		while ( (newResult > oldResult || newResult > olderResult) || newResult < 0.005);
		
		if ( !tunedIn ) continue;
		
		if (oldResult!=-1) tuneOffset--;
//		cout<<"Fix at: ["<<tuneOffset<<"] "<<oldResult<<endl<<flush;

		if (tuneOffset) memmove(bigBuffer, bigBuffer+tuneOffset, sizeof(short)*(bigBufferSize-tuneOffset) );
		bigBufferSize-=tuneOffset;
		
		while ( offset+sampleSize<=bigBufferSize )
		{
			memcpy(pieceBuffer, bigBuffer+offset, sampleSize*sizeof(short) );
			offset+=sampleSize;
			
//			cout<<"Debug: "<<endl;
//				for (int i=0; i<sampleSize; i++) cout<<pieceBuffer[i]<<"->"<<dbuffer[i]<<endl;
			
//			{
//				short lastSample = pieceBuffer[0];
//				
//				for (unsigned int i=1; i<sampleSize; i++)
//				{
//					if ( pieceBuffer[i] )
//						{
//							if ( lastSample*pieceBuffer[i]<0 ) std::cout<<"Cross at sample: "<<i<<endl<<flush;
//							lastSample = pieceBuffer[i];
//						}
//				}
//			}
			
			
			for (int i = 0; i < sampleSize; i++) dbuffer[i] = pieceBuffer[i] / 32768.0; //normalize
			
			double mark=0, space=0;
			
			mark = goertzel_mag(sampleSize, Freq[1], sampleRate, dbuffer);
			space = goertzel_mag(sampleSize, Freq[0], sampleRate, dbuffer);
			
//			cout<<"Decoding: "<<mark<<"|"<<space<<"["<<absolute(mark-space)<<"]"<<endl<<flush;

			if ( mark<0.005 && space<0.005 ) //|| absolute(mark-space)<0.025
			{
				tunedIn=0;
				
//				cout<<"Failed because: "<<mark<<"|"<<space<<endl;
//				for (int i=0; i<sampleSize; i++) cout<<pieceBuffer[i]<<"->"<<dbuffer[i]<<endl;
				break;
			}
			bool currentState = mark>space ? 1:0;
			
//			bool currentState=0;
			
//			unsigned int crosses = zeroX(pieceBuffer, sampleSize);
			
//			cout<<"Crosses: "<<crosses<<" 1=["<<mark<<"] 0=["<<space<<"]"<<endl<<flush;
			
//			if( crosses>3 && crosses<6 ) currentState = 0;
//			else if ( crosses>=6 && crosses<9 ) currentState=1;
//			else { tunedIn = 0; break; }
			
//			cout<<endl<<"DEBUG: ["<<mark<<"]["<<space<<"]"<<endl<<flush;
			resultingData.push_back( lastState==currentState ? 1:0 );
			lastState = currentState;
		}
		
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
