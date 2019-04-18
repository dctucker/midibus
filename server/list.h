#pragma once

#include <alsa/asoundlib.h>

struct device_map_t {
	char device[32];
	char name[64];
};

static void load_device_list(void);
void print_device_map();
const char *find_device(const char *);
