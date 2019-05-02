#pragma once

#include <alsa/asoundlib.h>
#include <limits.h>
#include "common.h"

#define MASK_SYSEX (1 << 20)
#define MASK_RT    (1 << 21)
#define MASK_ALL   (0x1fffe)

#define FILTER_CALLBACK(__name__) int __name__ (struct write_data *data, union write_args_t *args, unsigned char *buf, int n_bytes)

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
		char out_cc[128];
	} ccmap;
};

struct write_callback_t {
	int (*func)();
	union write_args_t args;
};

struct write_data {
	struct output_device_t *output_device;
	snd_rawmidi_t *midi_in;
	const char *port_name;
	const char *func_name;
	const char *args_name;
	struct write_callback_t callbacks[8];
};

void parse_write_args( struct write_data *, struct write_callback_t *, const char * );
struct write_callback_t *setup_write_func( struct write_data *, const char *func_name );
void teardown_write_func( struct write_data * );
void clear_write_data( struct write_data * );
ssize_t write_buffer(snd_rawmidi_t *, unsigned char *, size_t );
int (*str_write_func( const char *func_name ))();
