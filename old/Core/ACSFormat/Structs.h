#pragma once
#include "..\types.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <vector>

struct FrameImage;
struct AnimationInfo;
struct AnimationPointer;
struct BranchInfo;
struct OverlayInfo;
struct CharacterInfo;
struct VoiceInfo;
struct BalloonInfo;
struct TrayIcon;
struct IconImage;
struct BitmapInfoHeader;
struct StateInfo;
struct RGBQuad;
struct ACSLocator;
struct DataBlock;
struct CompressedData;
struct ACSHeader;
struct ImagePointer;
struct AudioPointer;
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

	void* RW;
	void* Chunk;
};

#pragma pack(push, 1)
struct RGBQuad {
	byte Blue;
	byte Green;
	byte Red;
	byte Reserved;
};

struct ACSLocator {
	uint Offset;
	uint Size;
};

struct DataBlock {
	uint SizeOfData;
	byte* Data;
};

struct CompressedData {
	uint CompressedSize;
	uint OriginalSize;
	byte* Data;
};

struct ACSHeader {
	uint Signature;
	ACSLocator CharacterInfo;
	ACSLocator AnimationInfo;
	ACSLocator ImageInfo;
	ACSLocator AudioData;
};

struct ImagePointer {
	ACSLocator LocationOfImage;
	uint Checksum;
};

struct AudioPointer {
	ACSLocator AudioData;
	uint CheckSum;
};

struct ImageInfo {
	byte Unknown;
	ushort Width;
	ushort Height;
	bool Compressed;
	DataBlock ImageData;
	CompressedData RegionData;
};

struct LocalizedInfo {
	ushort LanguageId {};
	string CharName {};
	string CharDescription {};
	string CharExtraData {};
};

struct FrameImage {
	uint FrameIndex {};
	short OffsetX {};
	short OffsetY {};
};

struct FrameInfo {
	std::vector<FrameImage> Images {};
	short AudioIndex {}; // -1 se inexistente
	ushort FrameDuration {};
	short ExitFrameIndex {};
	std::vector<BranchInfo> Branches {};
	std::vector<OverlayInfo> Overlays {};
};

struct AnimationInfo {
	string AnimationName {};
	TransitionType Transition {};
	string ReturnAnimation {};
	std::vector<FrameInfo> Frames {};
};

struct AnimationPointer { // list count: uint
	string AnimationName {};
	ACSLocator InfoLocation {};
};

struct BranchInfo {
	ushort TargetFrame {};
	ushort Probability {};
};

struct OverlayInfo {
	MouthOverlayType OverlayType {};
	bool ReplaceTopImage {};
	ushort ImageIndex {};
	byte Unknown {};
	bool HasRegionData {};
	short OffsetX {};
	short OffsetY {};
	ushort Width {};
	ushort Height {};
	DataBlock RegionData {};
};

struct GUID {
	unsigned long Data1 {};
	unsigned short Data2 {};
	unsigned short Data3 {};
	unsigned char Data4[8] {};
};

struct BitmapInfoHeader {
	uint StructSize {};
	int Width {};
	int Height {};
	ushort Planes {};
	ushort BitsPerPixel {};
	uint CompressionType {};
	uint SizeOfImageData {};
	int XPelsPerMeter {};
	int YPelsPerMeter {};
	uint ColorUsed {};
	uint ImportantColor {};
};

struct IconImage {
	BitmapInfoHeader IconHeader {};
	RGBQuad* ColorTable {};
	byte* Data {};
};

struct TrayIcon {
	uint SizeOfMonochromeData {};
	IconImage MonochromeBitmapData {};
	uint SizeOfColorData {};
	IconImage ColorBitmapData {};
};

struct ExtraVoiceInfo {
	ushort LangId {};
	string LanguageDialect {};
	ushort Gender {};
	ushort Age {};
	string Style {};
};

struct VoiceInfo {
	GUID TTSEngineId {}; // guid
	GUID TTSModeId {}; // guid
	uint Speed {};
	ushort Pitch {};
	bool ExtraData {};
};

struct BalloonInfo {
	byte TextLines {};
	byte CharsPerLine {};
	RGBQuad ForegroundColor {};
	RGBQuad BackgroundColor {};
	RGBQuad BorderColor {};
	string FontName {};
	int FontHeight {};
	int FontWeight {};
	bool Italic {};
	byte Unknown {};
};

struct StateInfo {
	string StateName {};
	std::vector<string> Animations {};
};

struct CharacterInfo {
	ushort MinorVersion {};
	ushort MajorVersion {};
	ACSLocator LocalizedInfo {};
	GUID CharId {};
	ushort Width {};
	ushort Height {};
	byte TransparentColorIndex {};
	CharacterFlags Flags {};
	ushort AnimationMajorVersion {};
	ushort AnimationMinorVersion {};
	VoiceInfo VoiceInfo {};
	ExtraVoiceInfo AdditionalVoiceInfo {};
	BalloonInfo BalloonInfo {};
	bool TrayIconEnabled {};
	TrayIcon SystemTrayIcon {};
	ushort AnimationStatesCount {};
	StateInfo* StateList {};
};

struct Rect {
	int x, y, w, h;

	const int bottom() {
		return y + h;
	}

	const int right() {
		return x + w;
	}
};
#pragma pack(pop)