#pragma once

#include <string>
#include <vector>
#include <memory>

#include "langid.h"
#include <span>

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

enum MouthOverlayType : uint8_t {
	Closed = 0,
	WideOpen1 = 1,
	WideOpen2 = 2,
	WideOpen3 = 3,
	WideOpen4 = 4,
	Medium = 5,
	Narrow = 6
};

enum TransitionType : uint8_t {
	ReturnAnimation = 0,
	ExitBranches = 1,
	None = 2
};

enum CharacterFlags : uint32_t {
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
	uint8_t Blue{};
	uint8_t Green{};
	uint8_t Red{};
	uint8_t Reserved{};
};

struct AgRect {
	int32_t Left{};
	int32_t Top{};
	int32_t Right{};
	int32_t Bottom{};
};

struct RgnDataHeader {
	uint32_t HeaderSize{};
	uint32_t Type{};
	uint32_t RectCount{};
	uint32_t BufferSize{};
	AgRect Bounds{};
};

struct RgnData {
	RgnDataHeader Header{};
	std::vector<AgRect> Rects;
};

struct LocalizedInfo {
	LangId LanguageId{};
	std::wstring CharName{};
	std::wstring CharDescription{};
	std::wstring CharExtraData{};
};

struct FrameImage {
	uint32_t ImageIndex{};
	short OffsetX{};
	short OffsetY{};
};

struct FrameInfo {
	std::vector<FrameImage> Images{};
	short AudioIndex{};
	uint16_t FrameDuration{};
	short ExitFrameIndex{};
	std::vector<BranchInfo> Branches{};
	std::vector<OverlayInfo> Overlays{};
};

struct AnimationInfo {
	std::wstring AnimationName{};
	TransitionType Transition{};
	std::wstring ReturnAnimation{};
	std::vector<FrameInfo> Frames{};
};

struct BranchInfo {
	uint16_t TargetFrame{};
	uint16_t Probability{};
};

struct OverlayInfo {
	MouthOverlayType OverlayType{};
	bool ReplaceTopImage{};
	uint16_t ImageIndex{};
	uint8_t Unknown{};
	bool HasRegionData{};
	int16_t OffsetX{};
	int16_t OffsetY{};
	uint16_t Width{};
	uint16_t Height{};
	RgnData RegionData{};
};

struct Guid {
	uint32_t Data1{};
	uint16_t Data2{};
	uint16_t Data3{};
	uint8_t Data4[8]{};
};

struct BitmapInfoHeader {
	uint32_t StructSize{};
	int32_t Width{};
	int32_t Height{};
	uint16_t Planes{};
	uint16_t BitsPerPixel{};
	uint32_t CompressionType{};
	uint32_t SizeOfImageData{};
	int32_t XPelsPerMeter{};
	int32_t YPelsPerMeter{};
	uint32_t ColorUsed{};
	uint32_t ImportantColor{};
};

struct IconImage {
	BitmapInfoHeader IconHeader{};

	std::shared_ptr<RGBQuad> ColorTable{};
	std::shared_ptr<uint8_t> PixelData;
};

struct TrayIcon {
	uint32_t SizeOfMaskData{};
	IconImage MaskBitmapData{};
	uint32_t SizeOfColorData{};
	IconImage ColorBitmapData{};
};

struct ExtraVoiceInfo {
	LangId LangId{};
	std::wstring LanguageDialect{};
	uint16_t Gender{};
	uint16_t Age{};
	std::wstring Style{};
};

struct VoiceInfo {
	Guid TTSEngineId{};
	Guid TTSModeId{};
	uint32_t Speed{};
	uint16_t Pitch{};
	bool ExtraData{};
};

struct BalloonInfo {
	uint8_t TextLines{};
	uint8_t CharsPerLine{};
	RGBQuad ForegroundColor{};
	RGBQuad BackgroundColor{};
	RGBQuad BorderColor{};
	std::wstring FontName{};
	int FontHeight{};
	int FontWeight{};
	bool Italic{};
	uint8_t Unknown{};
};

struct StateInfo {
	std::wstring StateName{};
	std::vector<std::wstring> Animations{};
};

struct CharacterInfo {
	uint16_t MinorVersion{};
	uint16_t MajorVersion{};
	Guid CharId{};
	uint16_t Width{};
	uint16_t Height{};
	uint8_t TransparencyIndex{};
	std::vector<RGBQuad> ColorTable{};
	CharacterFlags Flags{};
	uint16_t AnimationMajorVersion{};
	uint16_t AnimationMinorVersion{};
	bool HasTrayIcon{};
	VoiceInfo VoiceInfo{};
	ExtraVoiceInfo AdditionalVoiceInfo{};
	BalloonInfo BalloonInfo{};
};
#pragma pack(pop)

struct ImageData {
	uint16_t Width{};
	uint16_t Height{};

	std::span<uint8_t> Data{};
};

struct AgPoint {
	int32_t X;
	int32_t Y;
};