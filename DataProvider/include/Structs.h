#pragma once

#include <string>
#include <vector>
#include <memory>

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
struct RgnData;

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
	VoiceEnabled = 1 << 5,
	BalloonDisabled = 1 << 8,
	BalloonEnabled = 1 << 9,
	BalloonSizeToText = 1 << 16,
	BalloonAutoHideDisabled = 1 << 17,
	BalloonAutoPaceDisabled = 1 << 18,
	StandardAnimationSet = 1 << 20
};

#pragma pack(push, 1)
struct RGBQuad {
	byte Blue{};
	byte Green{};
	byte Red{};
	byte Reserved{};
};

struct AgRect {
	int32_t Left{};
	int32_t Top{};
	int32_t Right{};
	int32_t Bottom{};
};

struct RgnDataHeader {
	uint HeaderSize{};
	uint Type{};
	uint RectCount{};
	uint BufferSize{};
	AgRect Bounds{};
};

struct RgnData {
	RgnDataHeader Header{};
	std::vector<AgRect> Rects;
};

struct LocalizedInfo {
	ushort LanguageId{};
	string CharName{};
	string CharDescription{};
	string CharExtraData{};
};

struct FrameImage {
	uint ImageIndex{};
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
	RgnData RegionData{};
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
	byte TransparencyIndex{};
	std::vector<RGBQuad> ColorTable{};
	CharacterFlags Flags{};
	ushort AnimationMajorVersion{};
	ushort AnimationMinorVersion{};
	VoiceInfo VoiceInfo{};
	ExtraVoiceInfo AdditionalVoiceInfo{};
	BalloonInfo BalloonInfo{};
};
#pragma pack(pop)

struct AudioData {
	std::shared_ptr<byte> Data;
	uint Size;
};

struct ImageData {
	ushort Width{};
	ushort Height{};

	std::shared_ptr<byte> Data{};
	size_t Size{};
};