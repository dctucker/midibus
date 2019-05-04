#include "macro.h"
#include "app.h"

extern ssize_t write_buffer(snd_rawmidi_t *, unsigned char *, size_t );

void add_macro_listener( const char *port_name, const char *args )
{
	int i = 0;
	for(; i < app.n_read_threads; ++i )
		if( strcmp( app.read_data[i].port_name, port_name ) == 0 )
			break;
	if( i == app.n_read_threads )
	{
		printf("W device %s not found\n", app.read_data[i].port_name);
		return;
	}

	int m = 0;
	for(; m < MAX_MACROS; m++ )
		if( app.read_data[i].macros[m].name[0] == 0 )
			break;

	int comma = strchr(args, ',') - args;
	strncpy( app.read_data[i].macros[m].name, args, comma );
	char* copy = strdup(&args[comma+1]);

	int b = 0;
	char *pt = strtok(copy, " ");
	while( pt )
	{
		char data = (unsigned char)(strtoul( pt, NULL, 16 ));
		//printf("0x%X ", data);
		app.read_data[i].macros[m].data[b++] = data;
		pt = strtok(NULL, " ");
	}
	app.read_data[i].macros[m].n_data = b;
	printf("W %s macro listener %s\n", port_name, args);
}

void setup_macro( const char *port_name, const char *args )
{
	int m = 0;
	for(; m < MAX_MACROS; m++ )
		if( app.macros[m].port_name == NULL )
			break;

	int comma = strchr(args, ',') - args;
	app.macros[m].port_name = port_name;
	strncpy( app.macros[m].name, args, comma );
	char* copy = strdup(&args[comma+1]);

	memset( &app.macros[m].data, 0, BUFSIZE );
	int b = 0;
	char *pt = strtok(copy, " ");
	while( pt )
	{
		char data = (unsigned char)(strtoul( pt, NULL, 16 ));
		//printf("0x%X ", data);
		app.macros[m].data[b++] = data;
		pt = strtok(NULL, " ");
	}
	app.macros[m].n_data = b;
	printf("W %s macro %s\n", port_name, app.macros[m].name);
	free(copy);
}

void run_macro( const char *name )
{
	int m = 0;
	for(; m < MAX_MACROS; m++ )
		if( strncmp( app.macros[m].name, name, strlen(name)-1 ) == 0 )
			break;
	if( m == MAX_MACROS )
	{
		printf("N macro %s not found\n", name);
		return;
	}

	struct macro_data_t *macro = &app.macros[m];

	int o = 0;
	for(; o < app.n_output_devices; ++o )
		if( app.output_devices[o].port_name != NULL && strcmp( app.output_devices[o].port_name, macro->port_name ) == 0 )
			break;
	if( o == app.n_output_devices )
	{
		printf("N device %s not found\n", app.output_devices[o].port_name);
		return;
	}
	write_buffer(
		app.output_devices[o].midi,
		macro->data,
		macro->n_data
	);
	printf("%s\n", app.output_devices[o].port_name);
}

void hear_macro( struct macro_listener_t *macro, const char *port_name, unsigned char *buf, size_t len )
{
	if( len < macro->n_data )
		return;
	for( int b = 0; b < len; b++ )
	{
		if( strncmp( macro->data, buf, macro->n_data ) == 0 )
		{
			run_macro( macro->name );
			return;
		}
	}
}
