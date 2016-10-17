#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <math.h>

int rate = 8000;
int bps = 10;
FILE* infile = NULL;
FILE* outfile = NULL;

void usage(char* cmd) {
	printf("Usage: %s [-r sample rate ] [-b baud] [-s] [-h] [ -o file ] [ file ]\n" 
		"\t -r : Set signal sample rate (default = 8000)\n" 
		"\t -o : Write data to file\n"
		"\t -b : Set transferred bits per second\n" 
		"\t -h : Display this message\n\n", cmd);
}

void signal_generate(int length, float volume, ...) {
	va_list l;

	va_start(l, volume);
	signed char* signal = malloc(length);
	memset(signal, 0, length);	
	int frequency;
	while((frequency = va_arg(l, int)) > 0) {
		int i;
		for(i = 0; i < length; i++) {
			signal[i] += (int)(volume * 127.0 * sin(2 * M_PI * (float)frequency * i / (float) rate));
		}
	}

	fwrite(signal, 1, length, outfile);

	free(signal);

	va_end(l);

}

int state = 0;
void generate_nextbit() {
		signal_generate(4*rate/bps, 0.5, 400, 0);
		signal_generate(rate/bps, 0.0, 400, 0);
}
void generate_bit(int value) {
	if(value) {
		signal_generate(rate/bps, 0.5, 400, 0);
		signal_generate(rate/bps, 0.0, 400, 0);
	} else {
		signal_generate(2*rate/bps, 0.5, 400, 0);
		signal_generate(rate/bps, 0.0, 400, 0);
	}		
}

int main(int argc, char* argv[]) {
	infile = stdin;
	outfile = stdout;

	for(;;) switch(getopt(argc, argv, "r:hb:o:")) {
		case 'r':
			rate = atoi(optarg);
			break;
		case 'b': bps = atoi(optarg);
			break;
		case 'o':
			outfile = fopen(optarg, "w");
			if(outfile == NULL) {
				perror(optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h': usage(argv[0]);
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

	int c;
	while( ( c = fgetc(infile)) != EOF ) {
		generate_bit(c & 128);
		generate_bit(c & 64);
		generate_bit(c & 32);
		generate_bit(c & 16);
		generate_bit(c & 8);
		generate_bit(c & 4);
		generate_bit(c & 2);
		generate_bit(c & 1);
		generate_nextbit();
	}

	return EXIT_SUCCESS;

}
