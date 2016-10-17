#include <stdlib.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

int rate = 48000;
int fmt_signed = 1;
char* dsp = "/dev/dsp";
int wide = 0;

void initialize(int fd) {
	int arg = 0;

	if(wide) arg = AFMT_S16_LE;
	else arg = AFMT_S8;
	if(ioctl(fd, SNDCTL_DSP_SETFMT, &arg) == -1) {
		perror("SNDCTL_DSP_SETFMT");
		exit(EXIT_FAILURE);
	}

	arg = 1;
	if(ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg) == -1)  {
		perror("SNDCTL_DSP_STEREO");
		exit(EXIT_FAILURE);
	}
	if(arg != 1) {
		fprintf(stderr, "Unable to set mono mode\n");
		exit(EXIT_FAILURE);
	}

	arg = rate;
	if(ioctl(fd, SNDCTL_DSP_SPEED, &arg) == -1) { 
		perror("SNDCTL_DSP_SPEED");
		exit(EXIT_FAILURE);
	}
	if(arg != rate) {
		fprintf(stderr, "Unable to set rate\n");
		exit(EXIT_FAILURE);
	}

}
void usage(char* cmd) {
	fprintf(stderr, "usage: %s [-h] [-d dev] [-i outfile] [-r rate]\n"
			"\t -h : Display this message\n"
			"\t -d : Specify device (e.g. /dev/dsp)\n"
			"\t -i : Set input file (default stdin)\n"
			"\t -r : Set rate\n\n", cmd);
			
}

int main(int argc, char* argv[]) {
	FILE* infile = stdin;
	char* buffer = NULL;
	short* sbuffer = NULL;
	int fd = 0, i;

	while((i = getopt(argc, argv, "r:d:hi:w"))) switch(i) {
		case 'h': usage(argv[0]); return EXIT_SUCCESS;
		case 'r': rate = atoi(optarg); break;
		case 'd': dsp = strdup(optarg); break;
		case 'w': wide = 1; break;
		case 'i': 
			infile = fopen(optarg, "r"); 
			if(!infile) perror(optarg);
			break;
		case -1: goto done;
		default: exit(EXIT_FAILURE);
		
	}	
	done:
	buffer = malloc(rate);
	sbuffer = malloc(2*rate);

	fd = open(dsp, O_WRONLY);
	if(fd < 0) { perror(dsp); exit(EXIT_FAILURE); }
	initialize(fd);

	if(!wide)
	while(fread(buffer, 1, rate, infile) == rate) {
		if(!wide) {
			write(fd, buffer, rate);
		} else {
			int i;
			for(i = 0; i < rate; i++) {
				sbuffer[i] = buffer[i] * 0x100;
			}
			write(fd, sbuffer, 2*rate);
		}
	}

	return EXIT_SUCCESS;
}
