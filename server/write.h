#pragma once

#include <alsa/asoundlib.h>
#include <limits.h>
#include "common.h"

union write_args_t {
	void *pointer;
	struct channel_filter_data_t {
		int mask;
	} channel;
	struct funnel_filter_data_t {
		unsigned char channel;
	} funnel;
	struct ccmap_filter_data_t {
		unsigned char channel;
		char *out_cc;
	} ccmap;
};

struct write_data {
	struct output_device_t *output_device;
	snd_rawmidi_t *midi_in;
	const char *port_name;
	const char *func_name;
	const char *args_name;
	int (*func)();
	union write_args_t args;
};

void parse_write_args( struct write_data * );
void setup_write_func( struct write_data * );
void teardown_write_func( struct write_data * );
void clear_write_data( struct write_data * );
