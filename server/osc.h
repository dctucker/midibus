#pragma once

#include <stdio.h>
#include <lo/lo.h>
#include <pthread.h>
#include <unistd.h>

#define OSC_PORT 7770
#define OSC_PORT_STR "7770"

pthread_t *setup_osc();
