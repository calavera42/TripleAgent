#pragma once

#include "../include/agxstruct.h"

struct DataBlock {
	uint32_t SizeOfData;
	std::shared_ptr<uint8_t> Data;
};

struct ACSLocator {
	uint32_t Offset;
	uint32_t Size;
};

struct CompressedData {
	uint32_t CompressedSize{};
	uint32_t OriginalSize{};
	std::shared_ptr<uint8_t> Data;
};

struct AnimationPointer {
	std::wstring AnimationName{};
	ACSLocator InfoLocation{};
};

struct ImageInfo {
	uint8_t Unknown;
	uint16_t Width;
	uint16_t Height;
	bool Compressed;
	DataBlock ImageData;
	CompressedData RegionData;
};

struct ImagePointer {
	ACSLocator LocationOfImage;
	uint32_t Checksum;
};

struct AudioPointer {
	ACSLocator AudioData;
	uint32_t CheckSum;
};

struct ACSHeader {
	uint32_t Signature;
	ACSLocator CharacterInfo;
	ACSLocator AnimationInfo;
	ACSLocator ImageInfo;
	ACSLocator AudioData;
};