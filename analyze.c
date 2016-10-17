#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <math.h>

#include "fourier.h"

int rate = 8000;
int bps = 10;
int generate_xpm = 0;
double threshold = 0.25;

FILE* infile = NULL;
FILE* outfile = NULL;
FILE* xpmfile = NULL;

void usage(char* cmd) {
	printf("Usage: %s [-r sample rate] [-b baud] [-h] [-x] [ -o file ] [ file ]\n" 
		"\t -r : Set sample rate (default = 8000)\n" 
		"\t -o : Write data to file\n"
		"\t -x : Generate XPM with frequency spectrum\n"
		"\t -b : Set transferred bits per second\n" 
		"\t -h : Display this message\n\n", cmd);
}

int main(int argc, char* argv[]) {
	/* Set default arguments */
	infile = stdin;
	outfile = stdout;
	xpmfile = fopen("out.xpm", "w");

	for(;;) switch(getopt(argc, argv, "r:shb:o:x")) {
		case 'r':
			rate = atoi(optarg);
			break;
		case 'b': bps = atoi(optarg);
			break;
		case 'x':
			generate_xpm = 1;
			break;
		case 'o':
			outfile = fopen(optarg, "w+");
			if(outfile == NULL) {
				perror(optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h': usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		case -1: goto done;
		default: exit(EXIT_FAILURE);
	}

	done:


	if(optind < argc) {
		infile = fopen(argv[optind], "r");
		if(infile == NULL) {
			perror(argv[optind]);
			exit(EXIT_FAILURE);
		}
	}

	/* All-purpose index */
	int i = 0;

	/* Calculate file length */
	int file_length = 0;
	fseek(infile, 0, SEEK_END);
	file_length = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	/* The buffer length to use */
	int length = rate / (4 * bps);
	/* Equivalent of a 400 Hz signal */
	double frq = 400 * length / rate;

	/* Raw data buffer */
	char* buffer = malloc(length);

	/* Massaged data buffer */
	double* dbuffer = malloc(length);

	int signal_length = 0;
	int bit = 0;
	unsigned char bit_data = 0;

	if(generate_xpm) fourier_xpm_start(length, file_length / length, xpmfile);

	while((fread(buffer, 1, length, infile))) {
		double sum = 0;


		/* Transform into a double in the range -1 ... 1 */
		for (i = 0; i < length; i++) {
			dbuffer[i] = buffer[i] / 128.0;
		}
		
		if(generate_xpm) fourier_xpm_line(dbuffer, length, xpmfile);

		/* See the Shannon-Nyquist sampling theorem for why
		 * it's only meaningful to iterate over half the frequency
		 * spectrum */
		int harmonic;
		for(harmonic = 1; 2*harmonic < length / frq; harmonic++) 
			sum += fourier1(dbuffer, frq*harmonic, length);

		if(sum < threshold) { 
			if(signal_length) {
				if(signal_length > 10) {
					/* End of bit has been received
					 * but 8 full bits have not been 
					 * received. Strong indicator that
					 * we've gotten into corruption.
					 */
					if(bit != 0) printf("(?)");
					bit = 0;
					signal_length = 0;
				} else {
					bit_data = 2 * bit_data + (signal_length < 6);
					if(++bit == 8) {
						printf("%c", bit_data);
						fflush(NULL);
						bit = 0;
					} 
				}
				signal_length = 0;
			} 
		} else {
			signal_length++;
		}
	}
	return EXIT_SUCCESS;

}
