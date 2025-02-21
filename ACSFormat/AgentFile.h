#pragma once
#include "structs.h"

#include "BitReader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3_image/SDL_image.h>

#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

class AgentFile 
{
private:
	ACSHeader FileHeader = {};

	std::map<string, StateInfo> AnimationStates = {};
	std::map<string, AnimationPointer> Animations = {};

	std::vector<ImagePointer> ImagePointers = {};
	std::vector<AudioPointer> AudioPointers = {};

	std::map<ushort, LocalizedInfo> LocalizationInfo = {};

	SDL_Palette* CharacterPalette = nullptr;
	RGBQuad ColorKey = {};

	std::ifstream Stream;

	static string ReadString(std::ifstream& str);

	template <typename Type>
	static Type* ReadElements(std::ifstream& str, size_t count);

	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	template <typename Type>
	static Type ReadSimple(std::ifstream& str);

	void NormalizeString(string& s);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage();
	SDL_Surface* ReadTrayIcon();
	SDL_Surface* RasterizeIconImage(IconImage& im);
	void ReadAnimationInfo(ACSLocator* pos);
	void ReadImageInfo(ACSLocator* pos);
	void ReadAudioInfo(ACSLocator* pos);
	void DecompressData(void* inputBuffer, size_t inputSize, byte* outputBuffer);

public:
	SDL_Surface* AgentTrayIcon = {};
	void Load(std::string path);

	CharacterInfo CharInfo = {};

	LocalizedInfo* GetLocalizedInfo(ushort langId);

	StateInfo* ReadState(string name);
	AnimationInfo ReadAnimation(string name);

	SDL_Surface* ReadImage(uint index);

	AudioInfo ReadAudio(uint index);
};