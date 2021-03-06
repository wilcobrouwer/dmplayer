#include "filter_sampledoubler.h"

#include "../cross-platform.h"
#include <algorithm>
#include <stdexcept>

using namespace std;

SampleDoublerFilter::SampleDoublerFilter(IAudioSourceRef as)
	: IAudioSource(as->getAudioFormat()), src(as)
{
	if (audioformat.BitsPerSample * audioformat.Channels != 32) throw std::runtime_error("Works only with 32 bits per frame");
	audioformat.SampleRate *= 2;
}

uint32 SampleDoublerFilter::getData(uint8* buf, uint32 len)
{
	uint8 buffer[1024];
	uint32 res = 0;
	while (res < len) {
		int read = min((len-res)>>1, (uint32)1024);
		read = src->getData(buffer, read);
		if (!read) break;
		for (int i = 0; i < read; ) {
			buf[res++] = buffer[i++];
			buf[res++] = buffer[i++];
			buf[res++] = buffer[i++];
			buf[res++] = buffer[i++];
			buf[res++] = buffer[i-4];
			buf[res++] = buffer[i-3];
			buf[res++] = buffer[i-2];
			buf[res++] = buffer[i-1];
		}
	}
	return res;
}

bool SampleDoublerFilter::exhausted() {
	return src->exhausted();
}
