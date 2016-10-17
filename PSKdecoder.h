#ifndef PSKDECODER_H
#define PSKDECODER_H

#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <math.h>
#include <ctype.h>

#include "fourier.h"


class PSKdecoder
{
public:

unsigned int sampleRate;
unsigned int baud;
double threshold;
double carrier;

PSKdecoder()
{
	sampleRate = 44100;
	baud = 10;
	threshold = 0.01;
	carrier = 2000;
}

unsigned int decode(short *data, unsigned int length, char **result)
{
	/* The buffer length to use */
//	int length = sampleRate / (4 * baud);
	unsigned int sampleSize = sampleRate / baud;

	/* Raw data buffer */
	short* buffer = new short [sampleSize];

	/* Massaged data buffer */
	double* dbuffer = new double [sampleSize];

	char bit_data = 0;

	double old_carrier_phase[2] = { 0.0, 0.0 };
	
	unsigned int offset=0;
	while(  offset<length ) {
		
		if ( offset + sampleSize<=length ) memcpy(buffer, data+offset, sampleSize);
		else memcpy(buffer, data+offset, length-offset);
		
		offset+=sampleSize;
		
		/* Transform into a double in the range -1 ... 1 */
		for (int i = 0; i < sampleSize; i++)
		{
			dbuffer[i] = buffer[i] / 32767.0;
		}
		
//		double* debug = new double [sampleSize];
//		
//		fourier_w(dbuffer, debug, sampleSize );
//		
//		for (int i = 0; i < sampleSize/2; i++)
//		{
//			cout<<(int)((float)debug[i]*100)<<" "<<flush;
//		}
		
		
		double carrier_phase[2];
		double carrier_strength = fourier1p(dbuffer, (float) sampleSize * carrier / (float)sampleRate, sampleSize, &carrier_phase[0], &carrier_phase[1]);
		
		
		if(carrier_strength < threshold) continue;

//  -- this is pretty useful in debuging 
		printf("C: %f (%f %f)\n",
			carrier_strength, carrier_phase[0], carrier_phase[1]);


	/*	(A+Bi)*(C-Di) = A*C + B*D + i(-A*D + B*C) */
		double delta_re = carrier_phase[0] * old_carrier_phase[0] + carrier_phase[1]*old_carrier_phase[1];
		double delta_im = -carrier_phase[1]*old_carrier_phase[0] + carrier_phase[0] * old_carrier_phase[1];

		double realQ= delta_re * delta_re;
		double imaginaryQ = delta_im * delta_im;
		
		
		if ( realQ>0.33 && realQ<0.66 && imaginaryQ>0.33 && imaginaryQ<0.66)
			cout<<" END"<<endl;
		else if (realQ > imaginaryQ )
			cout<<"0";
		else if ( realQ < imaginaryQ )
			cout<<"1";

		
//		
//		if(realQ > imaginaryQ) { /* phase diff is a multiple of pi */
//			if(delta_re > 0); /* No change */
//			else {
//				bit_data = bit_data * 2;
//			}
//		} else {
//			if(delta_im > 0) {
//				bit_data = bit_data * 2 + 1;
//			} else {
//				if(isprint(bit_data)) printf("%c", bit_data);
//				else printf("<%.2x>", bit_data);
//				bit_data = 0;
//			}
//		}

		old_carrier_phase[0] = carrier_phase[0];
		old_carrier_phase[1] = carrier_phase[1];


	}
}

};

#endif