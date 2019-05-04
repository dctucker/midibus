#include "filters.h"

struct write_func_map_t write_func_map[] =
{
	FILTER_MAP( thru ),
	FILTER_MAP( realtime ),
	FILTER_MAP( funnel ),
	FILTER_MAP( ccmap ),
	FILTER_MAP( channel ),
	FILTER_MAP( status ),
	{"", NULL, NULL}
};

