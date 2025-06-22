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

class AgentFile : public IAgentFile
{
private:
	ACSHeader FileHeader = {};

	std::map<string, StateInfo> AnimationStates = {};
	std::map<string, AnimationPointer> Animations = {};

	std::vector<ImagePointer> ImagePointers = {};
	std::vector<AudioPointer> AudioPointers = {};

	std::map<ushort, LocalizedInfo> LocalizationInfo = {};

	CharacterInfo CharInfo = {};
	TrayIcon AgentIcon = {};

	bool Initialized = false;

	std::ifstream Stream;
public:
	//Entrada:
	// 	path - Caminho do arquivo do agente
	//
	//Saída:
	// 	0 - Sucesso
	// 	1 - Não foi possível abrir o arquivo
	// 	2 - Formato de arquivo inválido
	virtual int Load(std::string path) override;

	virtual CharacterInfo GetCharacterInfo() override;
	virtual LocalizedInfo GetLocalizedInfo(ushort langId) override;

	virtual StateInfo GetStateInfo(string name) override;
	virtual AnimationInfo GetAnimationInfo(string name) override;

	virtual ImageData ReadImageData(uint index) override;
	virtual AudioData ReadAudioData(uint index) override;

	virtual RgnData ReadImageRegion(unsigned int index) override;

	virtual TrayIcon GetAgentIcon() override;

	std::vector<std::wstring> GetAnimationNames() override;

private:
	template <typename Type>
	static Type* ReadElements(std::ifstream& str, size_t count);

	template <typename ListCount, typename Type>
	static std::vector<Type> ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc);

	template <typename Type>
	static Type ReadSimple(std::ifstream& str);

	static string ReadString(std::ifstream& str);
	static void NormalizeString(string& s);

	void ReadCharInfo(ACSLocator* pos);
	void ReadVoiceInfo();
	void ReadBalloonInfo();
	IconImage ReadIconImage(int dataSize);
	void ReadAnimationPointers(ACSLocator* pos);
	void ReadImagePointers(ACSLocator* pos);
	void ReadAudioPointers(ACSLocator* pos);
	RgnData ReadRegionData(CompressedData* cd);
	void DecompressData(byte* inputBuffer, size_t inputSize, byte* outputBuffer);
};