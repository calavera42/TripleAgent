#pragma once

#include <string>
#include <vector>

#ifdef DATAPROVIDER_EXPORTS
	#define AGENT_DPV __declspec(dllexport)
#else
	#define AGENT_DPV __declspec(dllimport)
#endif

typedef unsigned int uint;
typedef unsigned char byte;
typedef wchar_t wchar;
typedef unsigned short ushort;
typedef std::wstring string;

struct AnimationInfo;
struct BalloonInfo;
struct BitmapInfoHeader;
struct BranchInfo;
struct CharacterInfo;
struct FrameImage;
struct OverlayInfo;
struct VoiceInfo;
struct TrayIcon;
struct IconImage;
struct StateInfo;
struct RGBQuad;

struct DataBlock;
struct ImageInfo;

enum class MouthOverlayType : byte {
	Closed = 0,
	WideOpen1 = 1,
	WideOpen2 = 2,
	WideOpen3 = 3,
	WideOpen4 = 4,
	Medium = 5,
	Narrow = 6
};

enum class TransitionType : byte {
	ReturnAnimation = 0,
	ExitBranches = 1,
	None = 2
};

enum class CharacterFlags : uint {
	VoiceOutput = 1 << 5,
	BalloonEnabled = 1 << 9,
	BalloonSizeToText = 1 << 16,
	BalloonAutoHideDisabled = 1 << 17,
	BalloonAutoPaceDisabled = 1 << 18,
	StandardAnimationSet = 1 << 20
};

struct AudioData {
	void* Buffer;
	uint Size;
};

struct ImageData {
	ushort Width{};
	ushort Height{};

	void* Buffer{};
	size_t Size{};

	std::vector<RGBQuad>& ColorPalette;
};

#pragma pack(push, 1)
struct RGBQuad {
	byte Blue;
	byte Green;
	byte Red;
	byte Reserved;
};

struct DataBlock {
	uint SizeOfData;
	byte* Data;
};

struct LocalizedInfo {
	ushort LanguageId{};
	string CharName{};
	string CharDescription{};
	string CharExtraData{};
};

struct FrameImage {
	uint FrameIndex{};
	short OffsetX{};
	short OffsetY{};
};

struct FrameInfo {
	std::vector<FrameImage> Images{};
	short AudioIndex{};
	ushort FrameDuration{};
	short ExitFrameIndex{};
	std::vector<BranchInfo> Branches{};
	std::vector<OverlayInfo> Overlays{};
};

struct AnimationInfo {
	string AnimationName{};
	TransitionType Transition{};
	string ReturnAnimation{};
	std::vector<FrameInfo> Frames{};
};

struct BranchInfo {
	ushort TargetFrame{};
	ushort Probability{};
};

struct OverlayInfo {
	MouthOverlayType OverlayType{};
	bool ReplaceTopImage{};
	ushort ImageIndex{};
	byte Unknown{};
	bool HasRegionData{};
	short OffsetX{};
	short OffsetY{};
	ushort Width{};
	ushort Height{};
	DataBlock RegionData{};
};

struct Guid {
	unsigned long Data1{};
	unsigned short Data2{};
	unsigned short Data3{};
	unsigned char Data4[8]{};
};

struct BitmapInfoHeader {
	uint StructSize{};
	int Width{};
	int Height{};
	ushort Planes{};
	ushort BitsPerPixel{};
	uint CompressionType{};
	uint SizeOfImageData{};
	int XPelsPerMeter{};
	int YPelsPerMeter{};
	uint ColorUsed{};
	uint ImportantColor{};
};

struct IconImage {
	BitmapInfoHeader IconHeader{};
	RGBQuad* ColorTable{};
	byte* Data{};
};

struct TrayIcon {
	uint SizeOfMonochromeData{};
	IconImage MonochromeBitmapData{};
	uint SizeOfColorData{};
	IconImage ColorBitmapData{};
};

struct ExtraVoiceInfo {
	ushort LangId{};
	string LanguageDialect{};
	ushort Gender{};
	ushort Age{};
	string Style{};
};

struct VoiceInfo {
	Guid TTSEngineId{};
	Guid TTSModeId{};
	uint Speed{};
	ushort Pitch{};
	bool ExtraData{};
};

struct BalloonInfo {
	byte TextLines{};
	byte CharsPerLine{};
	RGBQuad ForegroundColor{};
	RGBQuad BackgroundColor{};
	RGBQuad BorderColor{};
	string FontName{};
	int FontHeight{};
	int FontWeight{};
	bool Italic{};
	byte Unknown{};
};

struct StateInfo {
	string StateName{};
	std::vector<string> Animations{};
};

struct CharacterInfo {
	ushort MinorVersion{};
	ushort MajorVersion{};
	Guid CharId{};
	ushort Width{};
	ushort Height{};
	CharacterFlags Flags{};
	ushort AnimationMajorVersion{};
	ushort AnimationMinorVersion{};
	VoiceInfo VoiceInfo{};
	ExtraVoiceInfo AdditionalVoiceInfo{};
	BalloonInfo BalloonInfo{};
};
#pragma pack(pop)

class IAgentFile
{
public:
	//Entrada:
	// 	path - Caminho do arquivo do agente
	//
	//Saída:
	// 	0 - Sucesso
	// 	1 - Não foi possível abrir o arquivo
	// 	2 - Formato de arquivo inválido
	virtual int Load(std::string path) = 0;

	virtual CharacterInfo GetCharacterInfo() = 0;
	virtual LocalizedInfo GetLocalizedInfo(unsigned short langId) = 0;

	virtual StateInfo GetStateInfo(std::wstring name) = 0;
	virtual AnimationInfo GetAnimationInfo(std::wstring name) = 0;

	virtual ImageData ReadImageData(unsigned int index) = 0;
	virtual AudioData ReadAudioData(unsigned int index) = 0;

	virtual std::vector<std::wstring> GetAnimationNames() = 0;
};

extern "C" AGENT_DPV IAgentFile* CreateAgentFile();
extern "C" AGENT_DPV void DeleteAgentFile(IAgentFile* agent);