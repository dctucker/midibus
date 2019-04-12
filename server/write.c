#include "write.h"
#include "thru.h"

// Note Off could be sent as Note On Vel 0
// Running status may need to be added when absent

ssize_t write_buffer(snd_rawmidi_t *port, unsigned char *buf, size_t n_bytes)
{
	if( n_bytes == 0 )
		return 0;
	ssize_t ret = snd_rawmidi_write( port, buf, n_bytes );
	printf("O ");
	for (int b = 0; b < n_bytes; ++b)
		printf("%02X ", buf[b]);
	return n_bytes;
}

int write_channel_filter(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	unsigned char out_buf[BUFSIZE];
	unsigned int mask = (int) args;
	unsigned int current_mask = UINT_MAX;
	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( buf[b] & 0x80 )
		{
			if( buf[b] < 0xf0 )
				current_mask = 2 << (buf[b] & 0x0f);
			else
				current_mask = UINT_MAX;
		}
		if( mask & current_mask )
			out_buf[a++] = buf[b];
	}
	// printf("Filter %x returned %d  ", mask, a); // debug
	return write_buffer( port, out_buf, a );
}

int write_thru(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	return write_buffer( port, buf, n_bytes );
}

int write_none(snd_rawmidi_t *port, unsigned char *buf, int n_bytes, void *args)
{
	return 0;
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
		printf("Setup channel mask %X\n", channel_mask);
	}
	else
	{
		data->func = write_none;
	}
	//printf("Setup write %s func %s args 0x%08x\n", data->port_name, name, data->args );
}
