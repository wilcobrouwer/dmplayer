#include "decoder_raw.h"
#include "../error-handling.h"
#include <string>

RawDecoder::RawDecoder()
{
	dcerr("");
	source = NULL;
}

RawDecoder::RawDecoder(IDataSource* source)
{
	dcerr("");
	this->source = source;
	//tryDecode(source);
}

RawDecoder::~RawDecoder()
{
	dcerr("");
}

IDecoder* RawDecoder::tryDecode(IDataSource* datasource)
{
	dcerr("");
	datasource->reset();
	char hdr[48];
	uint8* uhdr = (uint8*)hdr;
	if (datasource->read(hdr, 48) != 48) return NULL;
#define STRCHECK(x, str) for (unsigned int i = 0; i < strlen(str); ++i) if (hdr[x+i] != str[i]) return NULL;

	STRCHECK( 0, "RIFF");
	STRCHECK( 8, "WAVE");
	STRCHECK(12, "fmt ");
	STRCHECK(36, "data");

	uint16 fmt      = uhdr[20] + (uhdr[21] << 8);
	uint16 channels = uhdr[22] + (uhdr[23] << 8);
	uint32 srate    = uhdr[24] + (uhdr[25] << 8) + (uhdr[26] << 16) + (uhdr[27] << 24);
	uint16 bits     = uhdr[34] + (uhdr[35] << 8);

	if (fmt != 1)       return NULL;
	if (channels != 2)  return NULL; // TODO: mono->stereo filter?
	if (srate != 22050) return NULL; // TODO: samplerate conversion filter?
	if (bits != 16)     return NULL; // TODO: bps conversion filter?
	// or, return a decoder with the info from the header, so the controller
	// can change the backend output, or put in the conversion filters?

	return new RawDecoder(datasource);
}

uint32 RawDecoder::doDecode(char* buf, uint32 max, uint32 req)
{
	dcerr("");
	uint32 res = 0;
	do {
		uint32 read = source->read(buf, max);
		if (read == 0) return res;
		res += read;
	} while (res < req);
	return res;
}
