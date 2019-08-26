#pragma once

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <lo/lo.h>
#include <pthread.h>
#include <unistd.h>

#define OSC_PORT 7770
#define OSC_PORT_STR "7770"

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

pthread_t *setup_osc();
