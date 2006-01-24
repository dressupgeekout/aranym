/*
	Audio core

	ARAnyM (C) 2005 Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <SDL.h>

#include "host_audio.h"
#include "host.h"

#define DEBUG 0
#include "debug.h"

/*--- Defines ---*/

#define AUDIO_FREQ		22050
#define AUDIO_FORMAT	AUDIO_S16SYS
#define AUDIO_CHANNELS	2
#define	AUDIO_SAMPLES	1024

/*--- SDL callback function ---*/

extern "C" {
	static void UpdateAudio(void *unused, Uint8 *stream, int len) {
		DUNUSED(unused);
		HostAudio *hostAudio = host->getAudio();

		for (int i=0; i<MAX_AUDIO_CALLBACKS; i++) {
			if (hostAudio->callbacks[i]) {
				hostAudio->callbacks[i](hostAudio->userdatas[i], stream, len);
			}
		}
	}
};

/*--- Constructor/destructor of HostAudio class ---*/

HostAudio::HostAudio()
{
	D(bug("HostAudio: HostAudio()"));

	for (int i=0; i<MAX_AUDIO_CALLBACKS; i++) {
		callbacks[i]=NULL;
		userdatas[i]=NULL;
	}

	desired.freq = AUDIO_FREQ;
	desired.format = AUDIO_FORMAT;
	desired.channels = AUDIO_CHANNELS;
	desired.samples = AUDIO_SAMPLES;
	desired.callback = UpdateAudio;
	desired.userdata = NULL;

	if (SDL_OpenAudio(&desired, &obtained)<0) {
		fprintf(stderr,"Could not open audio: %s\n", SDL_GetError());
		return;
	}

	D(bug("HostAudio: %d Hz, 0x%04x format, %d channels", obtained.freq, obtained.format, obtained.channels));

	SDL_PauseAudio(0);
}

HostAudio::~HostAudio()
{
	D(bug("HostAudio: ~HostAudio()"));
	
	SDL_CloseAudio();
}

/*--- Public stuff ---*/

void HostAudio::reset(void)
{
	SDL_LockAudio();

	for (int i=0; i<MAX_AUDIO_CALLBACKS; i++) {
		callbacks[i]=NULL;
		userdatas[i]=NULL;
	}

	SDL_UnlockAudio();
}

void HostAudio::AddCallback(audio_callback_f callback, void *userdata)
{
	SDL_bool callbackAdded = SDL_FALSE;

	SDL_LockAudio();

	for (int i=0; i<MAX_AUDIO_CALLBACKS;i++) {
		if (callbacks[i]==NULL) {
			userdatas[i]=userdata;
			callbacks[i]=callback;
			callbackAdded = SDL_TRUE;
			break;
		}
	}

	SDL_UnlockAudio();

	if (!callbackAdded) {
		fprintf(stderr, "Too many audio callbacks registered\n");
		return;
	}
}

void HostAudio::RemoveCallback(audio_callback_f callback)
{
	SDL_LockAudio();

	for (int i=0; i<MAX_AUDIO_CALLBACKS;i++) {
		if (callbacks[i]==callback) {
			callbacks[i]=NULL;
			userdatas[i]=NULL;
		}
	}

	SDL_UnlockAudio();
}