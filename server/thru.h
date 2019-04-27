#pragma once

#include <alsa/asoundlib.h>
#include "write.h"
#include "common.h"

int n_read_threads;
int n_output_devices;

struct output_device_t {
	union {
		snd_rawmidi_t *midi;
		snd_rawmidi_t *midi_out;
	};
	snd_rawmidi_t *midi_in_exclusive;
	const char *port_name;
} output_devices[MAX_OUTS];

struct read_thread_data
{
	snd_rawmidi_t *midi;
	size_t n_outs;
	struct write_data outs[MAX_OUTS];
	const char *port_name;
} read_data[MAX_THREADS];


void manage_thread_outputs(struct read_thread_data *in);
void *read_thread(void *arg);
int setup_midi_device(struct read_thread_data *);
