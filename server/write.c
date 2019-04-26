#include "write.h"
#include "thru.h"

#define MASK_SYSEX (1 << 20)
#define MASK_RT    (1 << 21)
#define MASK_ALL   (0x1fffe)

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

int write_funnel_filter(struct write_data *data, unsigned char *buf, int n_bytes, void *args)
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char out_buf[BUFSIZE];
	unsigned char target = (unsigned char) (int) args;
	unsigned int mask = MASK_ALL; // any channel message is fine
	unsigned int current_mask = 0;
	if( data->output_device->midi_in_exclusive == data->midi_in )
		current_mask = MASK_SYSEX;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( buf[b] >= 0xf8 )
			current_mask = MASK_RT;
		else if( buf[b] >= 0xf0 )
		{
			if( buf[b] == 0xf0 )
				data->output_device->midi_in_exclusive = data->midi_in;
			else if( buf[b] == 0xf7 )
				data->output_device->midi_in_exclusive = NULL;
			current_mask = MASK_SYSEX;
		}
		else if( buf[b] >= 0x80 )
		{
			current_mask = 2 << (buf[b] & 0x0f);
			out_buf[a++] = buf[b] & 0xf0 | target;
			continue;
		}

		if( mask & current_mask )
		{
			out_buf[a++] = buf[b];
		}
	}
	return write_buffer( port, out_buf, a );
}

int write_channel_filter(struct write_data *data, unsigned char *buf, int n_bytes, void *args)
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char out_buf[BUFSIZE];
	unsigned int mask = (int) args;
	unsigned int current_mask = 0;
	if( data->output_device->midi_in_exclusive == data->midi_in )
		current_mask = MASK_SYSEX;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( buf[b] >= 0xf8 )
			current_mask = MASK_RT;
		else if( buf[b] >= 0xf0 )
		{
			if( buf[b] == 0xf0 )
				data->output_device->midi_in_exclusive = data->midi_in;
			else if( buf[b] == 0xf7 )
				data->output_device->midi_in_exclusive = NULL;
			current_mask = MASK_SYSEX;
		}
		else if( buf[b] >= 0x80 )
			current_mask = 2 << (buf[b] & 0x0f);

		if( mask & current_mask )
			out_buf[a++] = buf[b];
	}
	return write_buffer( port, out_buf, a );
}

int write_realtime(struct write_data *data, unsigned char *buf, int n_bytes, void *args)
{
	unsigned char out_buf[BUFSIZE];
	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( buf[b] == 0xf0 )
			data->output_device->midi_in_exclusive = data->midi_in;
		else if( buf[b] == 0xf7 )
			data->output_device->midi_in_exclusive = NULL;

		if( buf[b] & 0xF0 )
			out_buf[a++] = buf[b];
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}

int write_thru(struct write_data *data, unsigned char *buf, int n_bytes, void *args)
{
	return write_buffer( data->output_device->midi, buf, n_bytes );
}

int write_none(struct write_data *data, unsigned char *buf, int n_bytes, void *args)
{
	return 0;
}

void setup_write_func( struct write_data *data )
{
	if( strcmp(data->func_name, "thru") == 0 )
	{
		data->func = write_thru;
		printf("W %s thru\n", data->port_name, data->args_name);
	}
	else if( strcmp(data->func_name, "realtime") == 0 )
	{
		data->func = write_realtime;
		data->args = (void *) data->args_name;
		printf("W %s realtime %s\n", data->port_name, data->args_name);
	}
	else if( strcmp(data->func_name, "funnel") == 0 )
	{
		data->func = write_funnel_filter;
		data->args = (void *) ( atoi( data->args_name ) - 1 & 0x0f );
		printf("W %s funnel %s\n", data->port_name, data->args_name);
	}
	else if( strcmp(data->func_name, "channel") == 0 )
	{
		data->func = write_channel_filter;

		unsigned int channel_mask = 0;
		char* copy = strdup(data->args_name);
		char *pt;
		pt = strtok(copy, ",");
		while (pt != NULL)
		{
			int a = atoi(pt);
			if( a <= 0 )
			{
				if( strcmp(pt, "sysex") == 0 )
					channel_mask |= MASK_SYSEX;
				else if( strcmp(pt, "rt") == 0 )
					channel_mask |= MASK_RT;
				else if( strcmp(pt, "all") == 0 )
					channel_mask |= MASK_ALL;
			}
			else
				channel_mask |= 1 << a;

			pt = strtok(NULL, ",");
		}
		data->args = (void *) channel_mask;
		printf("W %s channel %X\n", data->port_name, channel_mask);
		free(copy);
	}
	else
	{
		data->func = write_none;
		printf("W none\b");
	}
	//printf("Setup write %s func %s args 0x%08x\n", data->port_name, name, data->args );
}
