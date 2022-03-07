#include "write.h"
#include "thru.h"
#include "filters/filters.h"

// Note Off could be sent as Note On Vel 0
// Running status may need to be added when absent

extern struct write_func_map_t write_func_map[];

unsigned char scan_status( struct write_data *data, unsigned char status)
{
	unsigned char *cur_status = &data->output_device->status;
	if( status < 0x80 )
		return *cur_status;
	*cur_status = status;
	return *cur_status;
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
		{
			printf(" callback #%d", c);
			return callback;
		}
	}
	for( int c = 0; c < MAX_CALLBACKS; c++ )
	{
		callback = &( data->callbacks[c] );
		if( callback->func == NULL )
		{
			printf(" %s new callback\n", func_name);
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
