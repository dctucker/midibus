#pragma once

#include "../write.h"
#include "../thru.h"

#define FILTER_CALLBACK(__name__) int callback_##__name__ (struct write_data *data, union write_args_t *args, unsigned char *buf, int n_bytes, unsigned char *out_buf)
#define FILTER_SETUP(__name__) void setup_##__name__ ( struct write_callback_t *callback, const char *args_name )
#define Q(x) #x
#define FILTER_MAP(__name__) { Q(__name__) , callback_##__name__,     setup_##__name__ }
#define FILTER_DEF(__name__) FILTER_SETUP(__name__); FILTER_CALLBACK(__name__)

extern unsigned char scan_status( struct write_data *data, unsigned char status);
extern ssize_t write_buffer(snd_rawmidi_t *port, unsigned char *buf, size_t n_bytes);

struct write_func_map_t {
	const char *name;
	int (*callback)();
	void (*setup)();
};

FILTER_DEF( ccmap );
FILTER_DEF( thru );
FILTER_DEF( channel );
FILTER_DEF( status );
FILTER_DEF( funnel );
FILTER_DEF( realtime );
FILTER_DEF( thru );
FILTER_DEF( none );
