#pragma once

#include "common.h"

struct macro_data_t {
	char name[32];
	const char *port_name;
	char data[BUFSIZE];
	unsigned int n_data;
};

struct macro_listener_t {
	char name[32];
	char data[16];
	unsigned int n_data;
};

void add_macro_listener( const char *port_name, const char *args );
