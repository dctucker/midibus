#pragma once

#include <alsa/asoundlib.h>
#include <limits.h>
#include "thru.h"
#include "common.h"

void setup_write_func( struct write_data *, char *, char *);
