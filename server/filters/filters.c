#include "filters.h"

struct func_map_t write_func_map[] =
{
	FILTER_MAP( thru ),
	FILTER_MAP( realtime ),
	FILTER_MAP( funnel ),
	FILTER_MAP( ccmap ),
	FILTER_MAP( channel ),
	FILTER_MAP( status ),
	{"", callback_none, NULL}
};

