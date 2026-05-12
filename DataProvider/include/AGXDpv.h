#pragma once

#ifdef DATAPROVIDER_EXPORTS
	#define AGENT_DPV __declspec(dllexport)
#else
	#define AGENT_DPV __declspec(dllimport)
#endif

#include <string>
#include <vector>

#include "agxstruct.h"

constexpr auto AGX_DPV_LOAD_SUCCESS = 0;
constexpr auto AGX_DPV_FAIL_TO_OPEN_STREAM = 1;
constexpr auto AGX_DPV_INVALID_FILE_SIGNATURE = 2;
constexpr auto AGX_DPV_INCOMPATIBLE_VERSION = 3;

enum class LoadResult
{
	Success,
	FailToOpenStream,
	InvalidSignature,
	IncompatibleVersion
};

class IAgentFile
{
public:
	IAgentFile() {};

	IAgentFile(const IAgentFile&) = delete;
	void operator=(const IAgentFile&) = delete;

	//Entrada:
	// 	path - Caminho do arquivo do agente
	virtual LoadResult Load(std::string path) = 0;

	virtual CharacterInfo GetCharacterInfo() = 0;
	virtual LocalizedInfo GetLocalizedInfo(LangId langId) = 0;

	virtual StateInfo GetStateInfo(std::wstring name) = 0;
	virtual AnimationInfo GetAnimationInfo(std::wstring name) = 0;

	virtual ImageData ReadImageData(uint32_t index) = 0;
	virtual std::span<uint8_t> ReadAudioData(uint32_t index) = 0;

	virtual RgnData ReadImageRegion(uint32_t index) = 0;

	virtual TrayIcon GetAgentIcon() = 0;

	virtual std::vector<std::wstring> GetAnimationsList() = 0;
	virtual std::vector<std::wstring> GetAvailableStates() = 0;
};

extern "C" AGENT_DPV IAgentFile* CreateAgentFile();
extern "C" AGENT_DPV void DestroyAgentFile(IAgentFile* agent);