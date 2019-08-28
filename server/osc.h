#pragma once

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <lo/lo.h>
#include <pthread.h>
#include <unistd.h>
#include "funcmap.h"

#define OSC_PORT 7770
#define OSC_PORT_STR "7770"

#define OSC_CALLBACK(__name__) int osc_callback_##__name__ (const char *path, const char *types, lo_arg **argv, int argc, void *msg, void *user_data)
#define OSC_SETUP(__name__) void setup_##__name__ ( const char *args_name )
#define OQ(x) #x
#define OSC_MAP(__name__) { OQ(__name__) , osc_callback_##__name__,     setup_##__name__ }
#define OSC_DEF(__name__) OSC_SETUP(__name__); OSC_CALLBACK(__name__)

pthread_t *setup_osc();

struct osc_map {
	const char *port_name;
	char ch;
	union {
		char cc;
		char cc_msb;
	};
	char cc_lsb;
	char values[32];
};

OSC_DEF( cc );
OSC_DEF( sw );
OSC_DEF( xy );
OSC_DEF( none );

