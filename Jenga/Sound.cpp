#include "Sound.h"

bool muted = false;

Sound::Sound(HWND hwnd)
{
	dSound = 0;
	primSoundBuffer = 0;
	soundBufferBGM = 0;
	
	
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	//init ponter interface
	DirectSoundCreate8(NULL, &dSound, NULL);

	dSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

	// primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	dSound->CreateSoundBuffer(&bufferDesc, &primSoundBuffer, NULL);

	// format of the primary sound bufffer.
	// still .WAV file - not sure if this'll work out with windows sounds
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
	
	primSoundBuffer->SetFormat(&waveFormat);

	loadMusic("beethovenmoonlight2ndpart.wav", &soundBufferBGM);
	//position at the beginning
	soundBufferBGM->SetCurrentPosition(0);

	for (int i = 0; i < numBlocks; i++)
	{
		loadMusic("drop.wav", &secSoundBufferBlock[i]);
	}

	loadMusic("falling.wav", &fallingSoundBuffer);
}

Sound::~Sound()
{
	soundBufferBGM->Release();

	for (int i = 0; i < numBlocks; i++)
	{
		secSoundBufferBlock[i]->Release();
	}

	fallingSoundBuffer->Release();

	primSoundBuffer->Release();
	dSound->Release();
}

void Sound::loadMusic(const char* name, IDirectSoundBuffer8** secBuffer)
{
	//load File
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	fopen_s(&filePtr, name, "rb");


	//check header for wrong fileformat
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if(count != 1)
		return;

	// Check that the chunk ID is the RIFF format.
	if((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') || 
	   (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
		return;

	// Check that the file format is the WAVE format.
	if((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
	   (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
		return;

	// Check that the sub chunk ID is the fmt format.
	if((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
	   (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
		return;

	// Check that the audio format is WAVE_FORMAT_PCM.
	if(waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
		return;

	// Check that the wave file was recorded in stereo format.
	if(waveFileHeader.numChannels != 2)
		return;

	// Check that the wave file was recorded at a sample rate of 44.1 KHz.
	if(waveFileHeader.sampleRate != 44100)
		return;

	// Ensure that the wave file was recorded in 16 bit format.
	if(waveFileHeader.bitsPerSample != 16)
		return;

	// Check for the data chunk header.
	if((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
	   (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
		return;


	// wave format of secondary buffer
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	//description of the secondary sound buffer
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	dSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);

	//test buffer and create it
	tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secBuffer);

	tempBuffer->Release();
	tempBuffer = 0;

	//find first address
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	waveData = new unsigned char[waveFileHeader.dataSize];

	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if(count != waveFileHeader.dataSize)
		return;

	fclose(filePtr);

	//get wave into secondary buffer
	(*secBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);
	(*secBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);

	//delete obsolete
	delete [] waveData;
	waveData = 0;
}

void Sound::playGameMusic()
{
	//muted = false;
	//play the bloody thing
	// set volume to 100%.
	//soundBufferBGM->SetVolume(DSBVOLUME_MAX);
	
	// play and loop
	//soundBufferBGM->Play(0, 0, DSBPLAY_LOOPING);

}

	//void Sound::playForebodingGameMusic();
	//void Sound::playMenueMusic();

void Sound::playCollisionSF(int num)
{
	if (!muted)
	{
		secSoundBufferBlock[num]->SetCurrentPosition(0);
		secSoundBufferBlock[num]->SetVolume(DSBVOLUME_MAX);
		secSoundBufferBlock[num]->Play(0, 0, 0);
	}

}

void Sound::playFallingSF()
{
	if (!muted)
	{
		fallingSoundBuffer->SetCurrentPosition(0);
		fallingSoundBuffer->SetVolume(DSBVOLUME_MAX);
		fallingSoundBuffer->Play(0, 0, 0); 
	}
}

void Sound::toggleMusic()
{
	//let play & mute 
	//soundBufferBGM->SetVolume(DSBVOLUME_MIN); 

	//muted = true;

	/* Pausing
	soundBufferBGM->Stop(); //*/

	/* Stopping
	soundBufferBGM->Stop();
	soundBufferBGM->SetCurrentPosition(0);

	//*/

	if (muted)
	{
		playGameMusic();
		muted = false;
	}
	else
	{
		soundBufferBGM->Stop();
		soundBufferBGM->SetCurrentPosition(0);
		muted = true;
	}
}