#include "write.h"
#include "thru.h"

int write_channel_filter(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	unsigned char out_buf[BUFSIZE];
	int a = 0;
	unsigned int mask = (int) args;
	unsigned int current_mask = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( buf[b] & 0x80 )
		{
			if( buf[b] < 0xf0 )
				current_mask = 2 << (buf[b] & 0x0f);
			else
				current_mask = UINT_MAX;
		}
		else // data
		{
			if( mask & current_mask == 0 )
				continue;
		}
		out_buf[a++] = buf[b];
	}
	snd_rawmidi_write( port, out_buf, a );
}

int write_thru(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	snd_rawmidi_write( port, buf, n_bytes );
}

int write_none(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	return 0;
}

int (*write_func(const char *name))()
{
	if( strcmp(name, "thru") == 0);
		return write_thru;
	return write_none;
}

void setup_write_func( struct write_data *data, char *name, char *args )
{
	if( strcmp(name, "thru") == 0 )
	{
		data->func = write_thru;
	}
	else if( strcmp(name, "channel") == 0 )
	{
		data->func = write_channel_filter;

		unsigned int channel_mask = 0;
		char *pt;
		pt = strtok(args, ",");
		while (pt != NULL)
		{
			int a = atoi(pt);
			channel_mask |= 1 << a;
			pt = strtok(NULL, ",");
		}
		data->args = (void *) channel_mask;
	}
	else
	{
		return;
	}
	//printf("Setup write %s func %s args 0x%08x\n", data->port_name, name, data->args );
}
