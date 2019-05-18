#pragma once

#include "thru.h"
#include "macro.h"

struct config_line {
	const char in[MAX_STRING];
	const char out[MAX_STRING];
	const char func[MAX_STRING];
	const char args[MAX_STRING];
};
extern struct config_line config[MAX_CONNECTIONS];

struct app_t {
	pthread_t threads[MAX_THREADS];

	int n_read_threads;
	int n_output_devices;
	struct read_thread_data read_data[MAX_THREADS];
	struct output_device_t output_devices[MAX_OUTS];
	struct config_line config[MAX_CONNECTIONS];
	struct macro_data_t macros[MAX_MACROS];
};
extern struct app_t *app;

extern void error(const char *, ...);
