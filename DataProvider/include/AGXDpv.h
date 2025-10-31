#pragma once

#ifdef DATAPROVIDER_EXPORTS
	#define AGENT_DPV __declspec(dllexport)
#else
	#define AGENT_DPV __declspec(dllimport)
#endif

#include <string>
#include <vector>

#include "Structs.h"

#define AGX_DPV_LOAD_SUCCESS 0
#define AGX_DPV_FAIL_TO_OPEN_STREAM 1
#define AGX_DPV_INVALID_FILE_SIGNATURE 2
#define AGX_DPV_INCOMPATIBLE_VERSION 3

class IAgentFile
{
public:
	IAgentFile() {};

	IAgentFile(const IAgentFile&) = delete;
	void operator=(const IAgentFile&) = delete;

	//Entrada:
	// 	path - Caminho do arquivo do agente
	//
	//Saída:
	// 	AGX_DPV_LOAD_SUCCESS
	// 	AGX_DPV_FAIL_TO_OPEN_STREAM
	// 	AGX_DPV_INCOMPATIBLE_VERSION
	virtual int Load(std::string path) = 0;

	virtual CharacterInfo GetCharacterInfo() = 0;
	virtual LocalizedInfo GetLocalizedInfo(unsigned short langId) = 0;

	virtual StateInfo GetStateInfo(std::wstring name) = 0;
	virtual AnimationInfo GetAnimationInfo(std::wstring name) = 0;

	virtual ImageData ReadImageData(unsigned int index) = 0;
	virtual AudioData ReadAudioData(unsigned int index) = 0;

	virtual RgnData ReadImageRegion(unsigned int index) = 0;

	virtual TrayIcon GetAgentIcon() = 0;

	virtual std::vector<std::wstring> GetAnimationsList() = 0;
	virtual std::vector<std::wstring> GetAvailableStates() = 0;
};

extern "C" AGENT_DPV IAgentFile* CreateAgentFile();
extern "C" AGENT_DPV void DestroyAgentFile(IAgentFile* agent);