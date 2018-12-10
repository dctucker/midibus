#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <pthread.h>

#define MAX_THREADS 16

int stop_all = 0;

struct write_data
{
	snd_rawmidi_t *midi;
	const char *port_name;
	int (*func)();
};

struct read_thread_data
{
	snd_rawmidi_t *midi;
	size_t n_outs;
	struct write_data outs[16];
	const char *port_name;
} read_data[MAX_THREADS];

