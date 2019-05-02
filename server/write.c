#include "write.h"
#include "thru.h"

// Note Off could be sent as Note On Vel 0
// Running status may need to be added when absent

unsigned char scan_status( struct write_data *data, unsigned char status)
{
	unsigned char *cur_status = &data->output_device->status;
	if( status < 0x80 )
		return *cur_status;
	*cur_status = status;
	return *cur_status;
}

FILTER_SETUP( ccmap )
{
	char* copy = strdup(args_name);
	unsigned char in_cc;
	char *pt;
	if( callback->args.ccmap.out_cc[127] == 0 )
		memset(callback->args.ccmap.out_cc, 0xff, 128);
	pt = strtok(copy, ","); callback->args.ccmap.channel = strtoul( pt, NULL, 10 ) - 1;
	pt = strtok(NULL, ","); in_cc = strtoul( pt, NULL, 16 );
	pt = strtok(NULL, ","); callback->args.ccmap.out_cc[in_cc] = strtoul( pt, NULL, 16 );
	printf("ccmap %d CC %d to %d",
		callback->args.ccmap.channel,
		in_cc,
		callback->args.ccmap.out_cc[in_cc]
	);
	free(copy);
}
FILTER_CALLBACK( ccmap )
{
	unsigned char channel = args->ccmap.channel;
	unsigned char cur_state;
	unsigned char exp_state = 0xb0 + channel;
	unsigned char cc = 0xff, val = 0xff;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
		if( buf[b] == exp_state )
			continue;

		if( cur_state == exp_state )
		{
			if(       cc == 0xff )  cc = buf[b];
			else if( val == 0xff ) val = buf[b];
			else
			{
				out_buf[a++] = cur_state;
				out_buf[a++] = cc;
				out_buf[a++] = val;
				cc = 0xff;
				val = 0xff;
			}
		}
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}

FILTER_SETUP( status )
{
	char* copy = strdup(args_name);
	char *pt;
	printf("status", args_name);

	pt = strtok(copy, ",");
	while (pt != NULL)
	{
		unsigned char a = strtoul( pt, NULL, 16 );
		callback->args.status.out_status[ a - 0x80 ] = a;
		printf(" 0x%X", a);
		pt = strtok(NULL, ",");
	}
	free(copy);
}
FILTER_CALLBACK( status )
{
	unsigned char *out_status = args->status.out_status;

	int a = 0;
	for( int b = 0; b < n_bytes; ++b )
	{
		unsigned char cur_state = scan_status(data, buf[b]);
		if( out_status[ cur_state - 0x80 ] == cur_state )
		{
			out_buf[a++] = buf[b];
		}
	}
	return write_buffer( data->output_device->midi, out_buf, a );
}

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

FILTER_SETUP( thru )
{
	printf("thru", args_name);
}
FILTER_CALLBACK( thru )
{
	for( int b = 0; b < n_bytes; ++b )
		scan_status(data, buf[b]);
	return write_buffer( data->output_device->midi, buf, n_bytes );
}

FILTER_SETUP( none )
{
	printf("none");
}
FILTER_CALLBACK( none )
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
		printf("0x%02X ", buf[b]);
	return n_bytes;
}

///////////////////////////////////////////////////////////////////////////////

struct write_func_map_t {
	const char *name;
	int (*callback)();
	void (*setup)();
} write_func_map[] = {
	FILTER_MAP( thru ),
	FILTER_MAP( realtime ),
	FILTER_MAP( funnel ),
	FILTER_MAP( ccmap ),
	FILTER_MAP( channel ),
	FILTER_MAP( status ),
	{"", NULL, NULL}
};

void parse_write_args( struct write_callback_t *callback, const char *args_name )
{
	int f = 0;
	do
	{
		struct write_func_map_t *map = &write_func_map[f];
		if( map->callback == callback->func )
		{
			map->setup( callback, args_name );
			printf("\n");
			return;
		}
		f++;
	}
	while( write_func_map[f].setup != NULL );
	printf("\n");
}

int (*str_write_func( const char *func_name ))()
{
	int f = 0;
	do
	{
		struct write_func_map_t *map = &write_func_map[f];
		if( strcmp(func_name, map->name) == 0 )
			return map->callback;
		f++;
	}
	while( write_func_map[f].callback != NULL );
	return callback_none;
}

struct write_callback_t *setup_write_func( struct write_data *data, const char *func_name )
{
	int (*func)() = str_write_func( func_name );

	printf("W %s ", data->port_name);

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
