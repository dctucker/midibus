#pragma once

struct func_map_t {
	const char *name;
	int (*callback)();
	void (*setup)();
};

int (*str_func( struct func_map_t *func_map, const char *func_name ))();
