#include "filters.h"

FILTER_SETUP( realtime )
{
	printf("realtime", args_name);
}
FILTER_CALLBACK( realtime )
{
	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
		if( buf[b] == 0xf0 )
			data->output_device->midi_in_exclusive = data->midi_in;
		else if( buf[b] == 0xf7 )
			data->output_device->midi_in_exclusive = NULL;

		if( buf[b] & 0xF0 )
			out_buf[a++] = buf[b];
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}
