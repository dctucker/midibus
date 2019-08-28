#include <string.h>
#include "funcmap.h"

int (*str_func( struct func_map_t *func_map, const char *func_name ))()
{
	int f = 0;
	do
	{
		struct func_map_t *map = &func_map[f];
		if( strcmp(func_name, map->name) == 0 )
			return map->callback;
		f++;
	}
	while( strlen( func_map[f].name ) != 0 );
	return func_map[f].callback;
}


