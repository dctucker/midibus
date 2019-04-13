#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

pthread_t threads[MAX_THREADS];
#define MAX_CONNECTIONS 64
#define MAX_STRING 256
