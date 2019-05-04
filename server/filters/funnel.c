#include "filters.h"

FILTER_SETUP( funnel )
{
	callback->args.funnel.channel = atoi( args_name ) - 1 & 0x0f;
	printf("funnel %s", args_name);
}
FILTER_CALLBACK( funnel )
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char target = (unsigned char) args->funnel.channel;
	unsigned int mask = MASK_ALL; // any channel message is fine
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
