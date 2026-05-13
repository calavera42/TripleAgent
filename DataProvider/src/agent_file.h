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
constexpr uint32_t ACS_MINOR_VERSION = 2;

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

class AgentFile : public IAgentFile
{
private:
	ACSHeader m_fileHeader = {};

	std::map<std::wstring, StateInfo> m_animationStates = {};
	std::map<std::wstring, AnimationPointer> m_animations = {};

	std::vector<ImagePointer> m_imagePointers = {};
	std::vector<AudioPointer> m_audioPointers = {};

	std::map<LangId, LocalizedInfo> m_localizationInfo = {};

	CharacterInfo m_charInfo = {};
	TrayIcon m_agentIcon = {};

	bool m_initialized = false;

	std::ifstream m_stream;

public:

	virtual ~AgentFile() override
	{
		m_stream.close();
	}

	virtual LoadResult Load(std::string path) override;

	virtual CharacterInfo GetCharacterInfo() override;
	virtual LocalizedInfo GetLocalizedInfo(LangId langId) override;

	virtual StateInfo GetStateInfo(std::wstring name) override;
	virtual AnimationInfo GetAnimationInfo(std::wstring name) override;

	virtual ImageData ReadImageData(uint32_t index) override;
	virtual std::span<uint8_t> ReadAudioData(uint32_t index) override;

	virtual TrayIcon GetAgentIcon() override;

	virtual std::vector<std::wstring> GetAnimationsList() override;
	virtual std::vector<std::wstring> GetAvailableStates() override;

private:
	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	static std::wstring ReadString(std::ifstream& str);
	static void NormalizeString(std::wstring& s);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage();
	void ReadAnimationPointers(ACSLocator* pos);
	void ReadImagePointers(ACSLocator* pos);
	void ReadAudioPointers(ACSLocator* pos);
	bool DecompressData(const std::vector<uint8_t>& inputBuffer, std::vector<uint8_t>& out);
};