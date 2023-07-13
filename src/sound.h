
// @Note(tkap, 13/07/2023): Needed for memcmp
#include <string.h>
#include "schnitzel_lib.h"

static constexpr int NUM_CHANNELS = 2;
static constexpr int SAMPLE_RATE = 44100;
static constexpr int MAX_CONCURRENT_SOUNDS = 16;

// @Note(tkap, 13/07/2023): For .wav loading
#pragma pack(push, 1)
struct s_riff_chunk
{
	unsigned int chunkId;
	unsigned int chunkSize;
	unsigned int format;
};

struct s_fmt_chunk
{
	unsigned int subChunk1Id;
	unsigned int subChunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
};

struct s_data_chunk
{
	unsigned int subChunk2Id;
	unsigned int subChunk2Size;
};
#pragma pack(pop)

struct Sound
{
	int sampleCount;
	short* samples;
};

typedef bool (*PlaySoundFunc)(Sound);

Sound load_wav(char* path, BumpAllocator* allocator)
{

	Sound result = {};
	int fileSize = 0;
	char* data = read_file(path, &fileSize);
	if(!data) { return {}; }

	s_riff_chunk riff = *(s_riff_chunk*)data;
	data += sizeof(riff);
	s_fmt_chunk fmt = *(s_fmt_chunk*)data;
	SM_ASSERT(fmt.numChannels == NUM_CHANNELS, "We only support 2 channels for now!");
	SM_ASSERT(fmt.sampleRate == SAMPLE_RATE, "We only support 44100 sample rate for now!");
	data += sizeof(fmt);
	s_data_chunk data_chunk = *(s_data_chunk*)data;
	SM_ASSERT(memcmp(&data_chunk.subChunk2Id, "data", 4) == 0, "Bad .wav parsing, blame tkap!");
	data += 8;

	result.sampleCount = data_chunk.subChunk2Size / NUM_CHANNELS / sizeof(short);
	result.samples = (short*)bump_alloc(allocator, NUM_CHANNELS * sizeof(short) * result.sampleCount);
	memcpy(result.samples, data, result.sampleCount * NUM_CHANNELS * sizeof(short));

	return result;
}