#ifndef BACKEND_SDLMIXER_H
#define BACKEND_SDLMIXER_H

#include "backend_interface.h"

class SDLMixerBackend : public IBackend {
	public:
		SDLMixerBackend(IDecoder* dec);
		virtual ~SDLMixerBackend() {};
		void test_playback(const char* filename);
		private:
};

#endif//BACKEND_SDLMIXER_H