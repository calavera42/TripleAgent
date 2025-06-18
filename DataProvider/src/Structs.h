#pragma once

struct DataBlock {
	uint SizeOfData;
	std::shared_ptr<byte> Data;
};

struct ACSLocator {
	uint Offset;
	uint Size;
};

struct CompressedData {
	uint CompressedSize{};
	uint OriginalSize{};
	std::shared_ptr<byte> Data;
};

struct AnimationPointer {
	string AnimationName{};
	ACSLocator InfoLocation{};
};

struct ImageInfo {
	byte Unknown;
	ushort Width;
	ushort Height;
	bool Compressed;
	DataBlock ImageData;
	CompressedData RegionData;
};

struct ImagePointer {
	ACSLocator LocationOfImage;
	uint Checksum;
};

struct AudioPointer {
	ACSLocator AudioData;
	uint CheckSum;
};

struct ACSHeader {
	uint Signature;
	ACSLocator CharacterInfo;
	ACSLocator AnimationInfo;
	ACSLocator ImageInfo;
	ACSLocator AudioData;
};