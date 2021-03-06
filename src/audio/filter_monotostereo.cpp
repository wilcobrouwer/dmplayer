#include "filter_monotostereo.h"

#include "../cross-platform.h"
#include "../error-handling.h"
#include <algorithm>

using namespace std;

MonoToStereoFilter::MonoToStereoFilter(IAudioSourceRef as)
	: IAudioSource(as->getAudioFormat()), src(as)
{
	if(audioformat.Channels != 1
	|| audioformat.Float
	|| audioformat.BitsPerSample != 16
	) throw Exception("Unsupported input format!");
	audioformat.Channels = 2;
}

uint32 MonoToStereoFilter::getData(uint8* buf, uint32 len)
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
			buf[res++] = buffer[i-2];
			buf[res++] = buffer[i-1];
		}
	}
	return res;
}

bool MonoToStereoFilter::exhausted() {
	return src->exhausted();
}
