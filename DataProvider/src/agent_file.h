#pragma once

#include "../include/agxdpv.h"
#include "acsstructs.h"

#include "bit_reader.h"

#include <functional>
#include <fstream>
#include <iostream>
#include <istream>

#include <map>
#include <vector>
#include <algorithm>

constexpr uint32_t ACS_MAGIC = 0xABCDABC3;
constexpr uint32_t ACS_MAJOR_VERSION = 2;

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

class AgentFile : public IAgentFile
{
private:
	ACSHeader FileHeader = {};

	std::map<wstring, StateInfo> _animationStates = {};
	std::map<wstring, AnimationPointer> _animations = {};

	std::vector<ImagePointer> _imagePointers = {};
	std::vector<AudioPointer> _audioPointers = {};

	std::map<LangId, LocalizedInfo> _localizationInfo = {};

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
	virtual LoadResult Load(std::string path) override;

	virtual CharacterInfo GetCharacterInfo() override;
	virtual LocalizedInfo GetLocalizedInfo(LangId langId) override;

	virtual StateInfo GetStateInfo(wstring name) override;
	virtual AnimationInfo GetAnimationInfo(wstring name) override;

	virtual ImageData ReadImageData(uint32_t index) override;
	virtual std::span<uint8_t> ReadAudioData(uint32_t index) override;

	virtual RgnData ReadImageRegion(uint32_t index) override;

	virtual TrayIcon GetAgentIcon() override;

	virtual std::vector<std::wstring> GetAnimationsList() override;
	virtual std::vector<std::wstring> GetAvailableStates() override;

private:
	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	template <typename Type>
	static Type ReadSimple(std::ifstream& str);

	static wstring ReadString(std::ifstream& str);
	static void NormalizeString(wstring& s);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage();
	void ReadAnimationPointers(ACSLocator* pos);
	void ReadImagePointers(ACSLocator* pos);
	void ReadAudioPointers(ACSLocator* pos);
	RgnData ReadRegionData(CompressedData* cd);
	bool DecompressData(const std::vector<uint8_t>& inputBuffer, std::vector<uint8_t>& out);
};