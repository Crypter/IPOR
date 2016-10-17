#ifndef ALSAPLAYBACK_H
#define ALSAPLAYBACK_H

#include <alsa/asoundlib.h>
#include <thread>
#include <mutex>
#include <queue>

#define PCM_DEVICE "default"

using namespace std;

class ALSAplayback
{
private:

struct bufferPiece
	{
		short *content=0;
		unsigned int length=0;
	};


	int pcm;
	unsigned int rate, channels, tmp;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	thread playingThread;
	mutex enqueue, playback;
	bool playbackOutrun;
	queue<bufferPiece> bufferQueue;

public:
ALSAplayback()
{
	rate 	 = 48000;
	channels = 1;
	playbackOutrun=0;
}

void init()
{
	/* Open the PCM device in playback mode */
	if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0) 
		printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

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

	printf("Playback state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

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
	
	snd_pcm_prepare(pcm_handle);
}

void addToQueue(short *buffer, unsigned int length)
{
	enqueue.lock();
	
	short *tempBuf = new short[length];
	memcpy(tempBuf, buffer, length*sizeof(short));
	bufferPiece temp;
	temp.content=tempBuf;
	temp.length = length;
	bufferQueue.push(temp);
	if (playbackOutrun==1)
	{
		playbackOutrun=0;
		playback.unlock();
	}
	
	enqueue.unlock();
}

void run()
{
	
}

void play(short *buffer, unsigned int length)
{
	
	pcm = snd_pcm_drain(pcm_handle);
	snd_pcm_prepare(pcm_handle);
//usleep(1000);
//		if (snd_pcm_avail(pcm_handle)>=length) cout<<"Buffer free:"<<snd_pcm_avail(pcm_handle)<<endl<<flush;

		if (pcm = snd_pcm_writei(pcm_handle, buffer, length) == -EPIPE) {
//			printf("XRUN.\n");
			snd_pcm_prepare(pcm_handle);
		} else if (pcm < 0) {
			printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
		}
		

}

void stop()
{
	
	pcm = snd_pcm_close(pcm_handle);
//	snd_pcm_hw_params_free(params);
}
	
	
};

#endif