#pragma once

#include <alsa/asoundlib.h>

int write_thru(snd_rawmidi_t *port, unsigned char *buf, int n_bytes);
int write_none(snd_rawmidi_t *port, unsigned char *buf, int n_bytes);
int (*write_func(const char *name))();
