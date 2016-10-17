#include <stdlib.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <memory.h>

int rate = 8000;
char* dsp = "/dev/dsp";
int wide = 0;

void initialize(int fd) {
	int arg = 0;

	if(!wide) arg = AFMT_S8;
	else arg = AFMT_S16_LE;

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
	fprintf(stderr, "usage: %s [-h] [-d dev] [-o outfile] [-r rate]\n"
			"\t -h : Display this message\n"
			"\t -d : Specify device (e.g. /dev/dsp)\n"
			"\t -o : Set output file (default stdout)\n"
			"\t -r : Set rate\n\n", cmd);
			
}
int main(int argc, char* argv[]) {
	FILE* outfile = stdout;
	int fd = 0;
	int i;
	char* buffer = NULL;
	short* sbuffer = NULL;

	while((i = getopt(argc, argv, "r:d:ho:w"))) switch(i) {
		case 'h': usage(argv[0]); return EXIT_SUCCESS;
		case 'r': rate = atoi(optarg); break;
		case 'd': dsp = strdup(optarg); break;
		case 'w': wide = 1; break;
		case 'o': 
			outfile = fopen(optarg, "w"); 
			if(!outfile) perror(optarg);
			break;
		case -1: goto done;
		default: exit(EXIT_FAILURE);
		
	}	
	done:
	buffer = malloc(rate);
	sbuffer = malloc(2*rate);

	fd = open(dsp, O_RDONLY);
	if(fd < 0) { perror(dsp); exit(EXIT_FAILURE); }
	initialize(fd);

	if(wide) {
		while(read(fd, sbuffer, 2*rate) == 2*rate) {
			for(i = 0; i < rate; i++) {
				buffer[i] = sbuffer[i] / 0x0100;
			}
			fwrite(buffer, 1, rate, outfile);
		}	
	} else {
		while(read(fd, buffer, rate) == rate) {
			fwrite(buffer, 1, rate, outfile);
		}
	}

	return EXIT_SUCCESS;
}
