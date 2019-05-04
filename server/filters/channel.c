#include "filters.h"

FILTER_SETUP( channel )
{
	unsigned int channel_mask = 0;
	char* copy = strdup(args_name);
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
	callback->args.channel.mask = channel_mask;
	printf("channel 0x%08X", channel_mask);
	free(copy);
}
FILTER_CALLBACK( channel )
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned int mask = (int) args->channel.mask;
	unsigned int current_mask = 0;
	if( data->output_device->midi_in_exclusive == data->midi_in )
		current_mask = MASK_SYSEX;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
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
