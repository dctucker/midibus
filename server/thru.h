#pragma once

#include <alsa/asoundlib.h>
#include "write.h"
#include "common.h"

struct output_device_t {
	union {
		snd_rawmidi_t *midi;
		snd_rawmidi_t *midi_out;
	};
	snd_rawmidi_t *midi_in_exclusive;
	const char *port_name;
};

struct read_thread_data {
	snd_rawmidi_t *midi;
	size_t n_outs;
	struct write_data outs[MAX_OUTS];
	const char *port_name;
};

void manage_thread_outputs(struct read_thread_data *in);
void *read_thread(void *arg);
int setup_midi_device(struct read_thread_data *);
