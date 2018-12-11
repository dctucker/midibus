#include "write.h"

int write_thru(snd_rawmidi_t *port, unsigned char *buf, int n_bytes)
{
	snd_rawmidi_write( port, buf, n_bytes );
}

int write_none(snd_rawmidi_t *port, unsigned char *buf, int n_bytes)
{
	return 0;
}

int (*write_func(const char *name))()
{
	if( strcmp(name, "thru") == 0);
		return write_thru;
	return write_none;
}


