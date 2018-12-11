#pragma once

#include <alsa/asoundlib.h>
#include "common.h"

int stop_all;
int n_read_threads;
int n_output_devices;

struct write_data
{
	snd_rawmidi_t *midi;
	const char *port_name;
	int (*func)();
	void *args;
};

struct read_thread_data
{
	snd_rawmidi_t *midi;
	size_t n_outs;
	struct write_data outs[MAX_OUTS];
	const char *port_name;
} read_data[MAX_THREADS];

struct output_device_t {
	snd_rawmidi_t *midi;
	const char *port_name;
} output_devices[MAX_OUTS];

void manage_thread_outputs(struct read_thread_data *in);
void *read_thread(void *arg);
void setup_midi_device(struct read_thread_data *);
