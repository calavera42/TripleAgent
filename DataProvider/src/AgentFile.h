#pragma once

#include "../include/AGXDpv.h"
#include "Structs.h"

#include "BitReader.h"

#include <functional>
#include <fstream>
#include <iostream>
#include <istream>

#include <map>
#include <vector>
#include <algorithm>

#define ACS_MAGIC 0xABCDABC3
#define ACS_MAJOR_VERSION 2

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

class AgentFile : public IAgentFile
{
private:
	ACSHeader FileHeader = {};

	std::map<string, StateInfo> _animationStates = {};
	std::map<string, AnimationPointer> _animations = {};

	std::vector<ImagePointer> _imagePointers = {};
	std::vector<AudioPointer> _audioPointers = {};

	std::map<ushort, LocalizedInfo> _localizationInfo = {};

	CharacterInfo _charInfo = {};
	TrayIcon _agentIcon = {};

	bool _initialized = false;

	std::ifstream _stream;

public:
	//Entrada:
	// 	path - Caminho do arquivo do agente
	//
	//Saída:
	// 	AGX_DPV_LOAD_SUCCESS
	// 	AGX_DPV_FAIL_TO_OPEN_STREAM
	// 	AGX_DPV_INCOMPATIBLE_VERSION
	virtual int Load(std::string path) override;

	virtual CharacterInfo GetCharacterInfo() override;
	virtual LocalizedInfo GetLocalizedInfo(ushort langId) override;

	virtual StateInfo GetStateInfo(string name) override;
	virtual AnimationInfo GetAnimationInfo(string name) override;

	virtual ImageData ReadImageData(uint index) override;
	virtual AudioData ReadAudioData(uint index) override;

	virtual RgnData ReadImageRegion(unsigned int index) override;

	virtual TrayIcon GetAgentIcon() override;

	virtual std::vector<std::wstring> GetAnimationsList() override;
	virtual std::vector<std::wstring> GetAvailableStates() override;

private:
	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	template <typename Type>
	static Type ReadSimple(std::ifstream& str);

	static string ReadString(std::ifstream& str);
	static void NormalizeString(string& s);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage();
	void ReadAnimationPointers(ACSLocator* pos);
	void ReadImagePointers(ACSLocator* pos);
	void ReadAudioPointers(ACSLocator* pos);
	RgnData ReadRegionData(CompressedData* cd);
	void DecompressData(byte* inputBuffer, size_t inputSize, byte* outputBuffer);
};