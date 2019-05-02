#include "write.h"
#include "thru.h"

// Note Off could be sent as Note On Vel 0
// Running status may need to be added when absent

FILTER_CALLBACK(write_ccmap_filter)
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char out_buf[BUFSIZE];
	int state = 0;
	unsigned char channel = args->ccmap.channel;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		if( state == 0 )
		{
			if( buf[b] == 0xb0 + channel )
			{
				++state;
			}
			else if( buf[b] == 0x80 + channel || buf[b] == 0x90 + channel || buf[b] == 0xe0 + channel )
			{
				out_buf[a++] = buf[b];
				state = 3;
			}
		}
		else if( state == 1 )
		{
			char in_cc = buf[b] & 0x7f;
			char out_cc = args->ccmap.out_cc[ in_cc ];
			if( out_cc >= 0x00 && out_cc < 0x7f )
			{
				out_buf[a++] = buf[ b - 1 ];
				out_buf[a++] = args->ccmap.out_cc[ in_cc ];
				++state;
			}
			else
			{
				state = 0;
			}
		}
		else if( state == 2 )
		{
			out_buf[a++] = buf[b];
			state = 0;
		}
		else if( state == 3 ) // note or pitch
		{
			out_buf[a++] = buf[b];
			state = 2;
		}
		
	}
	return write_buffer( port, out_buf, a );
}

FILTER_CALLBACK( write_funnel_filter )
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char out_buf[BUFSIZE];
	unsigned char target = (unsigned char) args->funnel.channel;
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

FILTER_CALLBACK(write_channel_filter)
{
	snd_rawmidi_t *port = data->output_device->midi;
	unsigned char out_buf[BUFSIZE];
	unsigned int mask = (int) args->channel.mask;
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

FILTER_CALLBACK( write_realtime )
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

FILTER_CALLBACK( write_thru )
{
	return write_buffer( data->output_device->midi, buf, n_bytes );
}

FILTER_CALLBACK( write_none )
{
	return 0;
}

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

///////////////////////////////////////////////////////////////////////////////

void parse_write_args( struct write_data *data, struct write_callback_t *callback, const char *args_name )
{
	if( callback->func == write_thru )
	{
		printf("W %s thru\n", data->port_name, args_name);
	}
	else if( callback->func == write_realtime )
	{
		printf("W %s realtime %s\n", data->port_name, args_name);
	}
	else if( callback->func == write_funnel_filter )
	{
		callback->args.funnel.channel = atoi( args_name ) - 1 & 0x0f;
		printf("W %s funnel %s\n", data->port_name, args_name);
	}
	else if( callback->func == write_ccmap_filter )
	{
		char* copy = strdup(args_name);
		unsigned char in_cc;
		char *pt;
		if( callback->args.ccmap.out_cc[127] == 0 )
			memset(callback->args.ccmap.out_cc, 0xff, 128);
		pt = strtok(copy, ","); callback->args.ccmap.channel = strtoul( pt, NULL, 10 ) - 1;
		pt = strtok(NULL, ","); in_cc = strtoul( pt, NULL, 16 );
		pt = strtok(NULL, ","); callback->args.ccmap.out_cc[in_cc] = strtoul( pt, NULL, 16 );
		printf("W %s ccmap %d CC %d to %d\n",
			data->port_name,
			callback->args.ccmap.channel,
			in_cc,
			callback->args.ccmap.out_cc[in_cc]
		);
		free(copy);
	}
	else if( callback->func == write_channel_filter )
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
		printf("W %s channel %X\n", data->port_name, channel_mask);
		free(copy);
	}
	else if( callback->func == write_none )
	{
		printf("W none\n");
	}
}

int (*str_write_func( const char *func_name ))()
{
	if( strcmp(func_name, "thru") == 0 )
		return write_thru;
	else if( strcmp(func_name, "realtime") == 0 )
		return write_realtime;
	else if( strcmp(func_name, "funnel") == 0 )
		return write_funnel_filter;
	else if( strcmp(func_name, "ccmap") == 0 )
		return write_ccmap_filter;
	else if( strcmp(func_name, "channel") == 0 )
		return write_channel_filter;
	else
		return write_none;
}

struct write_callback_t *setup_write_func( struct write_data *data, const char *func_name )
{
	int (*func)() = str_write_func( func_name );

	struct write_callback_t *callback = data->callbacks;
	for( int c = 0; c < MAX_CALLBACKS; c++ )
	{
		callback = &( data->callbacks[c] );
		if( callback->func == func )
			return callback;
	}
	for( int c = 0; c < MAX_CALLBACKS; c++ )
	{
		callback = &( data->callbacks[c] );
		if( callback->func == NULL )
		{
			//printf("W %s %s new callback\n", data->port_name, func_name);
			memset( callback, 0, sizeof( struct write_callback_t ) );
			callback->func = func;
			return callback;
		}
	}
	return NULL;
}

void clear_write_data( struct write_data *data )
{
	memset( data, 0, sizeof( struct write_data ) );
}

void teardown_write_func( struct write_data *data )
{
	// free memory from any args
	struct write_callback_t *callback = data->callbacks;
	/*
	struct write_callback_t *next = data->callbacks;
	do
	{
		next = callback->next;
		free( callback );
		callback = next;
	}
	while( callback != NULL );
	*/
	//memset( data, 0, sizeof( struct write_data ) );
	for( int c = 0; c < MAX_CALLBACKS; c++ )
	{
		callback = &data->callbacks[c];
		memset( callback, 0, sizeof( struct write_callback_t ) );
	}
}
