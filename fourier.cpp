#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "fourier.h"
#include <iostream>

/* alpha = RC / (RC + dt) 
 * cutoff frequency propto 1/RC
 *
 *  RC = 1 / f 
 *  dt = 1 / rate
 *  So alpha should be (rate / f) / ( rate / f + 1 / rate )
 **/

void lowpass(double x[], int length, double frq, double rate) {
	double alpha = (1 / frq) / ( (1 / frq) + 1 / rate );
	int i;
	for(i = 1; i < length; i++) {
		x[i] = alpha * x[i] + (1 - alpha)* x[i-1];
	}
}

void highpass(double x[], int length, double frq, double rate) {
	double alpha = (1 / frq) / ( (1 / frq) + 1 / rate );
	int i;
	double y[length];
	y[0] = x[0];
	for(i = 1; i < length; i++) {
		y[i] = alpha * (y[i-1] + x[i] - x[i-1]);
	}
	for(i = 0; i < length; i++) {
		x[i] = y[i];
	}
}

double fourier1(double x_in[], double n, int length) {
	double x_complex[2] = { 0, 0 };
	int i;

	for(i = 0; i < length; i++) {
		x_complex[0] += x_in[i] * cos(M_PI * 2 * i * n / (double) length);
		x_complex[1] += x_in[i] * sin(M_PI * 2 * i * n / (double) length);
	}

	return sqrt(x_complex[0]*x_complex[0] + x_complex[1]*x_complex[1]) / (double) length;
}

double fourier1p(double x_in[], double n, int length, double* phase_r, double* phase_i) {
	double x_complex[2] = { 0, 0 };
	int i;

	for(i = 0; i < length; i++) {
		x_complex[0] += x_in[i] * cos(M_PI * 2 * i * n / (double) length);
		x_complex[1] += x_in[i] * sin(M_PI * 2 * i * n / (double) length);
	}

	double norm = sqrt(x_complex[0]*x_complex[0] + x_complex[1]*x_complex[1]);
	*phase_i = x_complex[1] / norm;
	*phase_r = x_complex[0] / norm;
	return norm / length;
}
void fourier_w(double x_in[], double x_out[], int length) {
	int i;
	for(i = 0; i < length; i++) {
		x_out[i] = fourier1(x_in, i, length);
	}
}

void fourier_xpm_start(int seg_length, int file_length, FILE* output) {
	int i;
	fprintf(output, "! XPM2\n");
	fprintf(output,"%d %d 256 2\n", seg_length, file_length);

	for(i = 0; i < 256; i++) {
		fprintf(output, "%2.2x c #%2.2x%2.2x%2.2x\n", i, i, i, i);
	}
}

void fourier_xpm_line(double x_in[], int length, FILE* output) {
	int i;
	double x_out[length];
	fourier_w(x_in, x_out, length);
	
	
	for(i = 0; i < length; i++) {
		fprintf(output, "%2.2x", (unsigned char) (256 * x_out[i])); 
	}
	fprintf(output, "\n");
}

void blackman_harris(double *data, int numSamples)
{
	for (int n=0; n<numSamples; n++)
	{
			data[n] *= (double) 0.35875 - 0.48829*cos( 2*M_PI*n/(numSamples-1) ) + 0.14128*cos( 4*M_PI*n/(numSamples-1) ) - 0.01168*cos( 6*M_PI*n/(numSamples-1) );
	}
}

void hanning(double *data, int numSamples)
{
	for (int n=0; n<numSamples; n++)
	{
			data[n] *= (double) 0.5 * ( 1-cos( 2*M_PI*n/(numSamples-1) ) );
	}
}

void my_filter(double *data, int numSamples)
{
	for (int n=0; n<numSamples/5; n++)
	{
			data[n] *= (double) n/(numSamples/5.0);
			data[numSamples-n-1] *= (double) n/(numSamples/5.0);
	}
}

double goertzel_mag(int numSamples,int TARGET_FREQUENCY,int SAMPLING_RATE, double* data)
{
    int     k,i;
    double   floatnumSamples;
    double   omega,sine,cosine,coeff,q0,q1,q2,real,imag;

    double   scalingFactor = numSamples / 2.0;

    floatnumSamples = (double) numSamples;
    k = (int) (0.5 + ((floatnumSamples * TARGET_FREQUENCY) / SAMPLING_RATE));
    omega = (2.0 * M_PI * k) / floatnumSamples;
    sine = sin(omega);
    cosine = cos(omega);
    coeff = 2.0 * cosine;
    q0=0;
    q1=0;
    q2=0;

//	my_filter(data, numSamples);

    for(i=0; i<numSamples; i++)
    {
        q0 = coeff * q1 - q2 + data[i];
        q2 = q1;
        q1 = q0;
    }

    // calculate the real and imaginary results
    // scaling appropriately
    real = (q1 - q2 * cosine) / scalingFactor;
    imag = (q2 * sine) / scalingFactor;

    return sqrtf(real*real + imag*imag);
}

unsigned int zeroX(short *data, unsigned int length)
{
	unsigned int counter=0;
	
	short lastSample = data[0];
	
	for (unsigned int i=1; i<length; i++)
	{
		if ( data[i] )
			{
				if ( lastSample*data[i]<0 ) counter++;
				lastSample = data[i];
			}
	}
	return counter;
}