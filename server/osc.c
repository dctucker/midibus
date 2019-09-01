#include "osc.h"
#include "common.h"
#include "app.h"

struct func_map_t osc_func_map[] =
{
	OSC_MAP( cc ),
	OSC_MAP( sw ),
	OSC_MAP( xy ),
	OSC_MAP( note ),
	OSC_MAP( sysex ),
	{"", osc_callback_none, NULL}
};

pthread_t osc_thread;

lo_server_thread osc_server;
struct config_line osc_config[1024];
struct osc_map osc_maps[1024];
int num_maps = 0;

void osc_error(int num, const char *msg, const char *path)
{
	printf("E osc liblo server error %d in path %s:\n", num, path, msg);
	fflush(stdout);
}

int osc_default_handler(const char *path, const char *types, lo_arg **argv, int argc, void *msg, void *user_data)
{
	int i;
	printf("N osc %s ", path);
	for (i = 0; i < argc; i++) {
		printf(" %c:", types[i]);
		lo_arg_pp((lo_type)types[i], argv[i]);
	}
	printf("\n");
	fflush(stdout);
	return 1;
}

int osc_send_midi(const char *port_name, char *data, size_t len)
{
	int o = 0;
	for(; o < app->n_output_devices; ++o )
		if( app->output_devices[o].port_name != NULL && strcmp( app->output_devices[o].port_name, port_name ) == 0 )
			break;
	if( o == app->n_output_devices )
	{
		printf("N device %s not found\n", port_name);
		return 0;
	}
	write_buffer(
		app->output_devices[o].midi,
		data,
		len
	);
	printf("%s\n", app->output_devices[o].port_name);
	fflush(stdout);
}

OSC_CALLBACK( none )
{
	return 0;
}

OSC_SETUP( xy )
{
}
OSC_CALLBACK( xy )
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	unsigned short int y = argv[0]->f * 16383;
	unsigned short int x = argv[1]->f * 16383;
	char data[9];
	int i = 0;

	// x msb, lsb
	data[i++] = 0xb0 | ( map->ch & 0x0f );
	data[i++] = map->values[1];
	data[i++] = (x >> 7) & 0x7f;
	if( map->values[2] != -1 )
	{
		data[i++] = map->values[2];
		data[i++] = (x >> 0) & 0x7f;
	}

	// y msb, lsb
	data[i++] = map->values[3];
	data[i++] = (y >> 7) & 0x7f;
	if( map->values[4] != -1 )
	{
		data[i++] = map->values[4];
		data[i++] = (y >> 0) & 0x7f;
	}
	printf("V xy %i %i %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);

	osc_send_midi( map->port_name, data, i );
	fflush(stdout);
	return 0;
}

OSC_SETUP( note )
{
}
OSC_CALLBACK( note )
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	//int value = map->values[2];
	int value = 127;

	printf("V note port=%s cc=%d f=%f", map->port_name, map->cc, arg);


	printf(" v=%d", value);
	printf("\n");

	char data[6];
	size_t n_data = 3;
	data[0] = ( argv[0]->i == 0 ? 0x80 : 0x90 ) | ( map->ch & 0x0f );
	data[1] = map->cc & 0x7f;
	data[2] = value & 0x7f;

	osc_send_midi( map->port_name, data, n_data );
	fflush(stdout);
	return 0;
}

OSC_SETUP( sysex )
{
}
OSC_CALLBACK( sysex )
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	//int value = map->values[2];
	int value = 127;

	printf("V note port=%s cc=%d f=%f", map->port_name, map->cc, arg);

	if( argv[0]->i == 0 )
	{
		printf("\n");
		return 0;
	}

	printf("\n");

	char data[34];
	data[0] = 0xf0;
	int n = 0;
	for( ; data[n] != 0xff && n < 32; n++ )
	{
		data[n+1] = map->values[n];
	}
	data[n++] = 0xf7;

	osc_send_midi( map->port_name, data, n );
	fflush(stdout);
	return 0;
}

OSC_SETUP( sw )
{
}
OSC_CALLBACK( sw )
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	int value = map->values[2];

	printf("V sw port=%s cc=%d f=%f", map->port_name, map->cc, arg);

	if( argv[0]->i == 0 )
	{
		printf("\n");
		return 0;
	}

	printf(" v=%d", value);
	printf("\n");

	char data[6];
	size_t n_data = 3;
	data[0] = 0xb0 | ( map->ch & 0x0f );
	data[1] = map->cc & 0x7f;
	data[2] = value & 0x7f;

	osc_send_midi( map->port_name, data, n_data );
	fflush(stdout);
	return 0;
}

OSC_SETUP( cc )
{
}
OSC_CALLBACK( cc )
{
	struct osc_map *map = user_data;
	float arg = argv[0]->f;
	int value;

	printf("V port=%s cc=%d f=%f", map->port_name, map->cc, arg);

	value = (int)(arg * 127.0);
	printf(" v=%d", value);

	char data[6];
	size_t n_data = 3;
	data[0] = 0xb0 | ( map->ch & 0x0f );
	data[1] = map->cc & 0x7f;
	data[2] = value & 0x7f;
	printf("\n");

	osc_send_midi( map->port_name, data, n_data );

	return 0;
}

void osc_setup_handler(const char *port_name, const char *oscf, const char *vals, const char *path)
{
	int assigns;
	struct osc_map *map = &osc_maps[num_maps++];
	map->port_name = port_name;
	assigns = sscanf( vals, "%hhi,%hhi", &map->ch, &map->cc );
	int (*callback)() = str_func( osc_func_map, oscf );
	map->ch -= 1;
	printf("V setup %s %s@%x", map->port_name, oscf, callback);

	int n = 0;
	char *copy = strdup(vals);
	char *pt;
	pt = strtok( copy, "," );
	while( pt != NULL )
	{
		assigns = sscanf( pt, "%hhi", &map->values[n] );
		printf(" %d", map->values[n]);
		n++;
		pt = strtok( NULL, "," );
	}
	map->values[n] = 0xff;
	printf("\n");
	free(copy);
	lo_server_thread_add_method(osc_server, path, NULL, callback, map);
}

void osc_setup_handlers()
{
	unsigned int n = 0;
	FILE *fp = fopen("osc.conf","r");
	lo_server_thread_add_method(osc_server, NULL, NULL, osc_default_handler, NULL);
	while( fscanf( fp, "%s\t%s\t%s\t%[^\n]", osc_config[n].port, osc_config[n].oscf, osc_config[n].vals, osc_config[n].path) != EOF)
	{
		osc_setup_handler(osc_config[n].port, osc_config[n].oscf, osc_config[n].vals, osc_config[n].path);
		n++;
	}
	fclose(fp);
}

void osc_callback(const char *port_name, char *data, size_t data_len)
{
	printf("OSC got %s\n", port_name);

}

void *osc_run()
{
	osc_server = lo_server_thread_new(OSC_PORT_STR, osc_error);
	osc_setup_handlers();
	app->osc_callback = osc_callback;

	printf("N osc server listening on %s\n", OSC_PORT_STR);
	lo_server_thread_start(osc_server);

	while( ! stop_all )
	{
		sleep(1);
	}
	lo_server_thread_free(osc_server);
	printf("N osc server done\n");
	return 0;
}

pthread_t *setup_osc()
{
	pthread_create( &osc_thread, NULL, osc_run, NULL );
	return &osc_thread;
}
