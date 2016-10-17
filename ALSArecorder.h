#ifndef ALSARECORDER_H
#define ALSARECORDER_H

#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"

class ALSArecorder
{
private:
	int pcm;
	unsigned int rate, channels, tmp;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
//	snd_pcm_uframes_t frames;
	int loops;

public:
ALSArecorder()
{
	rate 	 = 48000;
	channels = 1;
}

void init()
{
	/* Open the PCM device in playback mode */
	if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0) < 0) 
		printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(pcm));

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) 
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE) < 0) 
		printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0) 
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0) 
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

	/* Write parameters */
	if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
//	printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

	printf("Recording state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	snd_pcm_hw_params_get_channels(params, &tmp);
	printf("channels: %i ", tmp);

	if (tmp == 1)
		printf("(mono)\n");
	else if (tmp == 2)
		printf("(stereo)\n");

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	printf("rate: %d bps\n", tmp);


	/* Allocate buffer to hold single period */
//	snd_pcm_hw_params_get_period_size(params, &frames, 0);

//	buff_size = frames * channels * 2 /* 2 -> sample size */;
//	buff = (char *) malloc(buff_size);

	snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
	
	pcm=snd_pcm_prepare(pcm_handle);
	
	pcm=snd_pcm_format_width(SND_PCM_FORMAT_S16_LE);
}

void record(short *buffer, unsigned int length)
{
	if (pcm = snd_pcm_readi(pcm_handle, buffer, length ) == -EPIPE) {
		printf("XRUN.\n");
	} else if (pcm < 0) {
		printf("ERROR. Can't read from PCM device. %s\n", snd_strerror(pcm));
	}
}

void destroy()
{
	pcm = snd_pcm_close(pcm_handle);
	snd_pcm_hw_params_free(params);
}
	
	
};

#endif