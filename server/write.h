#pragma once

#include <alsa/asoundlib.h>
#include <limits.h>
#include "thru.h"
#include "common.h"

enum status {
	NOTE_ON, NOTE_OFF, KEY_PRESSURE,
	CONTROL, PROGRAM, CHAN_PRESSURE,
	PITCH_BEND, SYSTEM
};

int message_lengths[] = { 2, 2, 2, 2, 1, 1, 2, -1 };

enum channel_mode {
	ALL_SOUND_OFF = 120,
	RESET_ALL,
	LOCAL_CONTROL,
	ALL_NOTES_OFF,
	OMNI_OFF,
	OMNI_ON,
	MONO,
	POLY,
};

enum system_common {
	SYSEX_START, TCQF, SONG_POS, SONG_SEL,
	UNDEF1, UNDEF2, TUNE_REQ, SYSEX_END,
	CLOCK, UNDEF3, START, CONTINUE,
	STOP, UNDEF4, ACTIVE_SENS, RESET
};

union midi_byte {
	unsigned char byte;
	struct {
		unsigned char is_status: 1;
		unsigned char status: 3;
		unsigned char channel: 4;
	};
	struct {
		unsigned char not_data: 1;
		unsigned char value: 7;
	};
};

void setup_write_func( struct write_data *, char *, char *);
