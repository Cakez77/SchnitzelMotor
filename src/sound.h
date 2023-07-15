#pragma once
#include "schnitzel_lib.h"

// #############################################################################
//                           Sound Constants
// #############################################################################
static constexpr int MAX_CONCURRENT_SOUNDS = 16;
static constexpr int MAX_ALLOCATED_SOUNDS = 64;
static constexpr int SOUNDS_BUFFER_SIZE = MB(128);

static constexpr float FADE_DURATION = 1.0f;

// #############################################################################
//                           Sound Structs
// #############################################################################
enum SoundOptionBits
{
	SOUND_OPTION_FADE_OUT = BIT(0),
	SOUND_OPTION_FADE_IN = BIT(1),
	SOUND_OPTION_START = BIT(0),
};
typedef int SoundOptions;

struct Sound
{
	char* path;
	float volume = 1.0f;
	SoundOptions options;
	int size;
	char* data;
};

struct SoundState
{
	// Buffer containing all Sounds
	int bytesUsed;
	char* allocatedsoundsBuffer;

	BumpAllocator* transientStorage;

	// Allocted sounds
	Array<Sound, MAX_CONCURRENT_SOUNDS> allocatedSounds;

	// Used by the platform to determine when to start and stop sounds
	Array<Sound, MAX_CONCURRENT_SOUNDS> playingSounds;
};

// #############################################################################
//                           Sound Globals
// #############################################################################
static SoundState* soundState;

// #############################################################################
//                           Sound Functions
// #############################################################################
void play_sound(Sound sound)
{
	// We can stop sounds using this function but if no
	// options are supplied, at least SOUND_OPTION_START
	// should be used.
	sound.options = sound.options? sound.options : SOUND_OPTION_START;

	// Look for existing Sound to play
	for(int soundIdx = 0; soundIdx < soundState->allocatedSounds.count; soundIdx++)
	{
		Sound allocatedSound = soundState->allocatedSounds[soundIdx];

		if(strcmp(allocatedSound.path, sound.path) == 0)
		{
			// Use allocated Sound
			allocatedSound.options = sound.options;
			soundState->playingSounds.add(allocatedSound);
			return;
		}
	}

	// Couldn't find a Sound, Load WAV file if presend and allocate
	WAVFile* wavFile = load_wav(sound.path, soundState->transientStorage);
	if(wavFile)
	{
		if(wavFile->header.dataChunkSize > SOUNDS_BUFFER_SIZE - soundState->bytesUsed)
		{
			SM_ASSERT(0, "Exausted Sounds Buffer!\nCapacity:\t%d\nBytes Used:\t%d\nSound Path:\t%s\nSound Size:\t%d",
									 SOUNDS_BUFFER_SIZE, soundState->bytesUsed, sound.path, wavFile->header.dataChunkSize);
			return;
		}
		sound.size = wavFile->header.dataChunkSize;
		sound.path = sound.path;
		sound.data = &soundState->allocatedsoundsBuffer[soundState->bytesUsed];
		soundState->bytesUsed += sound.size;
		memcpy(sound.data, &wavFile->dataBegin, sound.size);

		soundState->allocatedSounds.add(sound);
		soundState->playingSounds.add(sound);
	}
}

void stop_sound(Sound sound)
{
	sound.options = SOUND_OPTION_FADE_OUT;
	play_sound(sound);
}