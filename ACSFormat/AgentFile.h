#pragma once
#include "structs.h"

#include "BitReader.h"

#include <Windows.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_audio.h>
#include <SDL_mixer.h>
#include <SDL_syswm.h>

#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

class AgentFile 
{
private:
	ACSHeader FileHeader = {};
	TrayIcon AgentTrayIcon = {};

	std::map<string, StateInfo> AnimationStates = {};
	std::map<string, AnimationPointer> Animations = {};

	std::vector<ImagePointer> ImagePointers = {};
	std::vector<AudioPointer> AudioPointers = {};

	std::map<ushort, LocalizedInfo> LocalizationInfo = {};

	std::map<uint, SDL_Surface*> CachedSurfaces = {};

	SDL_Palette CharacterPalette = {};

	std::ifstream Stream;

	static string ReadString(std::ifstream& str);

	template <typename Type>
	static Type* ReadElements(std::ifstream& str, size_t count);

	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	template <typename Type>
	static Type ReadSimple(std::ifstream& str);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage();
	void ReadAnimationInfo(ACSLocator* pos);
	void ReadImageInfo(ACSLocator* pos);
	void ReadAudioInfo(ACSLocator* pos);
	void DecompressData(void* inputBuffer, size_t inputSize, byte* outputBuffer);
public:
	void Load(std::string path);

	CharacterInfo CharInfo = {};

	LocalizedInfo* GetLocalizedInfo(ushort langId);

	TrayIcon ReadTrayIcon();
	AnimationInfo ReadAnimation(string name);
	SDL_Surface* ReadImage(uint index);
	AudioInfo ReadAudio(uint index);
};