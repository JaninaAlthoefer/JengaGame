#pragma once

#include <stdio.h>
#include <dsound.h>
#include "GlobalSizes.h"

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")


class Sound
{

private:
	
	IDirectSound8* dSound;
	IDirectSoundBuffer* primSoundBuffer;
	IDirectSoundBuffer8* soundBufferBGM;
	IDirectSoundBuffer8* secSoundBufferBlock[numBlocks];
	IDirectSoundBuffer8* fallingSoundBuffer;


public:
	Sound(HWND hwnd);
	~Sound();

	void loadMusic(const char* name, IDirectSoundBuffer8** secondaryBuffer);
	void playGameMusic();
	//void playForebodingGameMusic();
	//void playMenueMusic();

	void playCollisionSF(int num);
	void playFallingSF();

	void toggleMusic();

};


//Sound Filetype structure
struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

