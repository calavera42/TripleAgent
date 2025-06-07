#include "AgentFile.h"

#define ReadTo(x, st) st.read((char*)&x, sizeof(x))
#define JumpTo(x, st) st.seekg(x, std::ios_base::beg)
#define Skip(x, st) st.seekg(x, std::ios_base::cur)

string AgentFile::ReadString(std::ifstream& str)
{
	int length = 0;
	wchar_t curChar = 0;
	string output = L"";

	ReadTo(length, str);

	if (length == 0)
		return output;

	for (size_t i = 0; i < length; i++)
	{
		ReadTo(curChar, str);
		output += curChar;
	}

	Skip(2, str); // null terminator (tem 2 bytes pq é um widechar)

	return output;
}

template <typename Type>
Type* AgentFile::ReadElements(std::ifstream& str, size_t count)
{
	Type* output = (Type*)calloc(count, sizeof(Type));

	if (output == nullptr)
		return nullptr;

	for (size_t i = 0; i < count; i++)
	{
		Type temp = {};
		ReadTo(temp, str);

		output[i] = temp;
	}

	return output;
}

template <typename ListCount, typename Type>
std::vector<Type> AgentFile::ReadVector(std::ifstream& str, std::function<Type(std::ifstream&)> readFunc)
{
	ListCount count = 0;
	std::vector<Type> output = {};

	if (readFunc == NULL)
		readFunc = ReadSimple<Type>;

	ReadTo(count, str);

	if (count == 0)
		return output;

	for (ListCount i = 0; i < count; i++)
		output.push_back(readFunc(str));

	return output;
}

template <typename Type>
Type AgentFile::ReadSimple(std::ifstream& str)
{
	Type temp = {};
	ReadTo(temp, str);

	return temp;
}

int AgentFile::Load(std::string path)
{
	Stream = std::ifstream(path, std::ios_base::binary | std::ios_base::in);

	if (!Stream.is_open())
		return 1;

	ReadTo(FileHeader, Stream);

	if (FileHeader.Signature != 0xABCDABC3)
		return 2;

	ReadCharInfo(&FileHeader.CharacterInfo);

	if (CharInfo.MajorVersion != 2)
		return 2;

	ReadAnimationPointers(&FileHeader.AnimationInfo);
	ReadImagePointers(&FileHeader.ImageInfo);
	ReadAudioPointers(&FileHeader.AudioData);

	Initialized = true;

	return 0;
}

CharacterInfo AgentFile::GetCharacterInfo()
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	return CharInfo;
}

void AgentFile::NormalizeString(string& s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
}

void AgentFile::ReadCharInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);

	ACSLocator localizedInfo = {};
	byte transparentColorIndex = 0;
	bool trayIconEnabled;

	ReadTo(CharInfo.MinorVersion, Stream);
	ReadTo(CharInfo.MajorVersion, Stream);
	ReadTo(localizedInfo, Stream);
	ReadTo(CharInfo.CharId, Stream);
	ReadTo(CharInfo.Width, Stream);
	ReadTo(CharInfo.Height, Stream);
	ReadTo(CharInfo.TransparencyIndex, Stream);
	ReadTo(CharInfo.Flags, Stream);
	ReadTo(CharInfo.AnimationMajorVersion, Stream);
	ReadTo(CharInfo.AnimationMinorVersion, Stream);

	ReadVoiceInfo();
	ReadBalloonInfo();

	CharInfo.ColorTable = ReadVector<uint, RGBQuad>(Stream, NULL);

	ReadTo(trayIconEnabled, Stream);

	if (trayIconEnabled) 
	{
		ReadTo(AgentIcon.SizeOfMonochromeData, Stream);
		AgentIcon.MonochromeBitmapData = ReadIconImage();

		ReadTo(AgentIcon.SizeOfColorData, Stream);
		AgentIcon.ColorBitmapData = ReadIconImage();
	}

	std::vector<StateInfo> states = ReadVector<ushort, StateInfo>(Stream, [](std::ifstream& str) -> StateInfo {
		StateInfo temp;

		temp.StateName = ReadString(str);
		temp.Animations = ReadVector<ushort, string>(str, ReadString);

		NormalizeString(temp.StateName);

		return temp;
	});

	for (auto& curState : states)
		AnimationStates.insert({ curState.StateName, curState });

	JumpTo(localizedInfo.Offset, Stream);
	std::vector<LocalizedInfo> locInfo = ReadVector<ushort, LocalizedInfo>(Stream, [](std::ifstream& str) -> LocalizedInfo {
		LocalizedInfo temp;

		ReadTo(temp.LanguageId, str);
		temp.CharName = ReadString(str);
		temp.CharDescription = ReadString(str);
		temp.CharExtraData = ReadString(str);

		return temp;
	});

	for (auto& loc : locInfo) 
		LocalizationInfo.insert({ loc.LanguageId, loc });
}

void AgentFile::ReadVoiceInfo()
{
	if (!((uint)CharInfo.Flags & (uint)CharacterFlags::VoiceEnabled))
		return;

	ReadTo(CharInfo.VoiceInfo, Stream);

	if (!CharInfo.VoiceInfo.ExtraData)
		return;

	CharInfo.AdditionalVoiceInfo = {};

	ReadTo(CharInfo.AdditionalVoiceInfo.LangId, Stream);
	CharInfo.AdditionalVoiceInfo.LanguageDialect = ReadString(Stream);
	ReadTo(CharInfo.AdditionalVoiceInfo.Gender, Stream);
	ReadTo(CharInfo.AdditionalVoiceInfo.Age, Stream);
	CharInfo.AdditionalVoiceInfo.Style = ReadString(Stream);
}

void AgentFile::ReadBalloonInfo()
{
	if (!((uint)CharInfo.Flags & (uint)CharacterFlags::BalloonEnabled))
		return;

	ReadTo(CharInfo.BalloonInfo.TextLines, Stream);
	ReadTo(CharInfo.BalloonInfo.CharsPerLine, Stream);

	ReadTo(CharInfo.BalloonInfo.ForegroundColor.Red, Stream);
	ReadTo(CharInfo.BalloonInfo.ForegroundColor.Green, Stream);
	ReadTo(CharInfo.BalloonInfo.ForegroundColor.Blue, Stream);
	ReadTo(CharInfo.BalloonInfo.ForegroundColor.Reserved, Stream);

	ReadTo(CharInfo.BalloonInfo.BackgroundColor.Red, Stream);
	ReadTo(CharInfo.BalloonInfo.BackgroundColor.Green, Stream);
	ReadTo(CharInfo.BalloonInfo.BackgroundColor.Blue, Stream);
	ReadTo(CharInfo.BalloonInfo.BackgroundColor.Reserved, Stream);

	ReadTo(CharInfo.BalloonInfo.BorderColor.Red, Stream);
	ReadTo(CharInfo.BalloonInfo.BorderColor.Green, Stream);
	ReadTo(CharInfo.BalloonInfo.BorderColor.Blue, Stream);
	ReadTo(CharInfo.BalloonInfo.BorderColor.Reserved, Stream);

	CharInfo.BalloonInfo.FontName = ReadString(Stream);
	ReadTo(CharInfo.BalloonInfo.FontHeight, Stream);
	ReadTo(CharInfo.BalloonInfo.FontWeight, Stream);
	ReadTo(CharInfo.BalloonInfo.Italic, Stream);
	ReadTo(CharInfo.BalloonInfo.Unknown, Stream);
}

LocalizedInfo AgentFile::GetLocalizedInfo(ushort langId)
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	if (LocalizationInfo.find(langId) == LocalizationInfo.end())
		return {};

	return LocalizationInfo[langId];
}

IconImage AgentFile::ReadIconImage() // leitura obrigatória: https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513
{
	IconImage iconImage = {};

	ReadTo(iconImage.IconHeader, Stream);
	iconImage.IconHeader.ColorUsed = 1 << (iconImage.IconHeader.BitsPerPixel * iconImage.IconHeader.Planes);

	iconImage.ColorTable = ReadElements<RGBQuad>(Stream, iconImage.IconHeader.ColorUsed);
	iconImage.Data = ReadElements<byte>(Stream, iconImage.IconHeader.SizeOfImageData);

	return iconImage;
}

void AgentFile::ReadAnimationPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);
	std::vector<AnimationPointer> animations = ReadVector<uint, AnimationPointer>(Stream, [](std::ifstream& str) -> AnimationPointer {
		AnimationPointer anim = {};
		anim.AnimationName = ReadString(str);
		ReadTo(anim.InfoLocation, str);

		std::transform(anim.AnimationName.begin(), anim.AnimationName.end(), anim.AnimationName.begin(), std::tolower);
		return anim;
	});

	for (auto& animPointer : animations)
		Animations.insert({ animPointer.AnimationName, animPointer });
}

AnimationInfo AgentFile::GetAnimationInfo(string name)
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);

	if (Animations.count(name) == 0)
		return {};

	AnimationPointer* pointer = &Animations[name];
	ACSLocator* pos = &pointer->InfoLocation;

	JumpTo(pos->Offset, Stream);

	AnimationInfo info = {};

	info.AnimationName = ReadString(Stream);
	ReadTo(info.Transition, Stream);
	info.ReturnAnimation = ReadString(Stream);
	info.Frames = ReadVector<ushort, FrameInfo>(Stream, [](std::ifstream& str) -> FrameInfo {
		FrameInfo fi = {};

		fi.Images = ReadVector<ushort, FrameImage>(str, NULL);

		ReadTo(fi.AudioIndex, str);
		ReadTo(fi.FrameDuration, str);
		ReadTo(fi.ExitFrameIndex, str);

		fi.Branches = ReadVector<byte, BranchInfo>(str, NULL);

		fi.Overlays = ReadVector<byte, OverlayInfo>(str, [](std::ifstream& str) -> OverlayInfo {
			OverlayInfo oi = {};

			ReadTo(oi.OverlayType, str);
			ReadTo(oi.ReplaceTopImage, str);
			ReadTo(oi.ImageIndex, str);
			ReadTo(oi.Unknown, str);
			ReadTo(oi.HasRegionData, str);
			ReadTo(oi.OffsetX, str);
			ReadTo(oi.OffsetY, str);
			ReadTo(oi.Width, str);
			ReadTo(oi.Height, str);

			if (oi.HasRegionData)
			{
				oi.RegionData = {};

				ReadTo(oi.RegionData.SizeOfData, str);
				oi.RegionData.Data = ReadElements<byte>(str, oi.RegionData.SizeOfData);
			}

			return oi;
		});

		return fi;
	});

	return info;
}

void AgentFile::ReadImagePointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);

	ImagePointers = ReadVector<uint, ImagePointer>(Stream, ReadSimple<ImagePointer>);
}

ImageData AgentFile::ReadImageData(uint index)
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= ImagePointers.size())
		return { 0, 0, nullptr, 0 };

	ImagePointer* imgPointer = &ImagePointers[index];
	ImageInfo imgInfo = {};

	JumpTo(imgPointer->LocationOfImage.Offset, Stream);
	ReadTo(imgInfo.Unknown, Stream);
	ReadTo(imgInfo.Width, Stream);
	ReadTo(imgInfo.Height, Stream);
	ReadTo(imgInfo.Compressed, Stream);
	ReadTo(imgInfo.ImageData.SizeOfData, Stream);

	imgInfo.ImageData.Data = (byte*)malloc(imgInfo.ImageData.SizeOfData);

	if (imgInfo.ImageData.Data == 0)
		return { 0, 0, nullptr, 0 };

	Stream.read((char*)imgInfo.ImageData.Data, imgInfo.ImageData.SizeOfData);

	size_t uncompressedImageSize = (size_t)imgInfo.Width * (size_t)imgInfo.Height;
	byte* uncompressedImage = (byte*)malloc(uncompressedImageSize);

	if (uncompressedImage == 0)
		return { 0, 0, nullptr, 0 };

	DecompressData(imgInfo.ImageData.Data, imgInfo.ImageData.SizeOfData, uncompressedImage);
	free(imgInfo.ImageData.Data);

	return { imgInfo.Width, imgInfo.Height, uncompressedImage, uncompressedImageSize };
}

AudioData AgentFile::ReadAudioData(uint index)
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	AudioPointer ap = AudioPointers[index];

	JumpTo(ap.AudioData.Offset, Stream);

	void* audioData = calloc(ap.AudioData.Size, 1);

	Stream.read((char*)audioData, ap.AudioData.Size);

	return { audioData, ap.AudioData.Size };
}

std::vector<string> AgentFile::GetAnimationNames()
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<string> names = {};

	for (const auto& animInfo : Animations)
		names.push_back(animInfo.first);

	return names;
}

void AgentFile::FreeImageData(ImageData& id)
{
	free(id.Buffer);
}

void AgentFile::FreeAudioData(AudioData& id)
{
	free(id.Buffer);
}

void AgentFile::ReadAudioPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);
	AudioPointers = ReadVector<uint, AudioPointer>(Stream, ReadSimple<AudioPointer>);
}

void AgentFile::DecompressData(void* inputBuffer, size_t inputSize, byte* outputBuffer)
{
	BitReader br = BitReader((byte*)inputBuffer, inputSize);

	const byte bitCountTable[] = {
		6, 9, 12, 20
	};

	const ushort valueSumTable[] = {
		1, 65, 577, 4673
	};

	uint index = 0;

	br.ReadBits(8); // padding rsrs

	while (true) 
	{
		if (!br.ReadBit()) // byte normal
		{ 
			outputBuffer[index] = (byte)br.ReadBits(8);
			index++;
			continue;
		}

		ushort countOfBytesToDecode = 2; // pelo menos 2 bytes codificados
		byte offsetSequentialBits = br.CountSequentialBits(3);
		byte offsetBitCount = bitCountTable[offsetSequentialBits];
		uint offset = br.ReadBits(offsetBitCount);

		if (offsetSequentialBits == 3) // 20 bits lidos
		{
			if (offset == 0x000FFFFF)
				break; // fim dos dados
			else
				countOfBytesToDecode++;
		}

		offset += valueSumTable[offsetSequentialBits];

		// a quantidade de bits sequenciais dita quantos bytes serão decodificados
		byte decBytesSequentialBits = br.CountSequentialBits(11); // o máximo de bits é 11

		if (decBytesSequentialBits == 11 && br.ReadBit()) // um bit extra que sempre deve ser 0
			throw ("O 12º bit da sequência é 1.");

		if (decBytesSequentialBits)
		{
			countOfBytesToDecode += (ushort)((1 << decBytesSequentialBits) - 1);
			countOfBytesToDecode += (ushort)br.ReadBits(decBytesSequentialBits);
		}

		for (int i = 0; i < countOfBytesToDecode; i++) // copiar os dados para o buffer de saída
		{
			outputBuffer[index] = outputBuffer[index - offset];
			index++;
		}
	}
}

StateInfo AgentFile::GetStateInfo(string name)
{
	if (!Initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);
	if (!AnimationStates.count(name))
		return {};

	return AnimationStates[name];
}
