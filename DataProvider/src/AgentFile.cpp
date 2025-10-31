#include "AgentFile.h"

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

	Skip(sizeof(wchar_t), str); // null terminator

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
	_stream = std::ifstream(path, std::ios_base::binary | std::ios_base::in);

	if (!_stream.is_open())
		return AGX_DPV_FAIL_TO_OPEN_STREAM;

	ReadTo(FileHeader, _stream);

	if (FileHeader.Signature != ACS_MAGIC)
		return AGX_DPV_INVALID_FILE_SIGNATURE;

	ReadCharInfo(&FileHeader.CharacterInfo);

	if (_charInfo.MajorVersion != ACS_MAJOR_VERSION)
		return AGX_DPV_INCOMPATIBLE_VERSION;

	ReadAnimationPointers(&FileHeader.AnimationInfo);
	ReadImagePointers(&FileHeader.ImageInfo);
	ReadAudioPointers(&FileHeader.AudioData);

	_initialized = true;

	return AGX_DPV_LOAD_SUCCESS;
}

CharacterInfo AgentFile::GetCharacterInfo()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	return _charInfo;
}

void AgentFile::NormalizeString(string& s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
}

void AgentFile::ReadCharInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);

	ACSLocator localizedInfo = {};

	ReadTo(_charInfo.MinorVersion, _stream);
	ReadTo(_charInfo.MajorVersion, _stream);
	ReadTo(localizedInfo, _stream);
	ReadTo(_charInfo.CharId, _stream);
	ReadTo(_charInfo.Width, _stream);
	ReadTo(_charInfo.Height, _stream);
	ReadTo(_charInfo.TransparencyIndex, _stream);
	ReadTo(_charInfo.Flags, _stream);
	ReadTo(_charInfo.AnimationMajorVersion, _stream);
	ReadTo(_charInfo.AnimationMinorVersion, _stream);

	ReadVoiceInfo();
	ReadBalloonInfo();

	_charInfo.ColorTable = ReadVector<uint, RGBQuad>(_stream, NULL);

	ReadTo(_charInfo.HasTrayIcon, _stream);

	if (_charInfo.HasTrayIcon)
	{
		ReadTo(_agentIcon.SizeOfMaskData, _stream);
		_agentIcon.MaskBitmapData = ReadIconImage();

		ReadTo(_agentIcon.SizeOfColorData, _stream);
		_agentIcon.ColorBitmapData = ReadIconImage();
	}

	std::vector<StateInfo> states = ReadVector<ushort, StateInfo>(_stream, [](std::ifstream& str) -> StateInfo {
		StateInfo temp;

		temp.StateName = ReadString(str);
		temp.Animations = ReadVector<ushort, string>(str, ReadString);

		NormalizeString(temp.StateName);

		return temp;
	});

	for (auto& curState : states)
		_animationStates.insert({ curState.StateName, curState });

	JumpTo(localizedInfo.Offset, _stream);
	std::vector<LocalizedInfo> locInfo = ReadVector<ushort, LocalizedInfo>(_stream, [](std::ifstream& str) -> LocalizedInfo {
		LocalizedInfo temp;

		ReadTo(temp.LanguageId, str);
		temp.CharName = ReadString(str);
		temp.CharDescription = ReadString(str);
		temp.CharExtraData = ReadString(str);

		return temp;
	});

	for (auto& loc : locInfo) 
		_localizationInfo.insert({ loc.LanguageId, loc });
}

void AgentFile::ReadVoiceInfo()
{
	if (!(_charInfo.Flags & CharacterFlags::VoiceEnabled))
		return;

	ReadTo(_charInfo.VoiceInfo, _stream);

	if (!_charInfo.VoiceInfo.ExtraData)
		return;

	_charInfo.AdditionalVoiceInfo = {};

	ReadTo(_charInfo.AdditionalVoiceInfo.LangId, _stream);
	_charInfo.AdditionalVoiceInfo.LanguageDialect = ReadString(_stream);
	ReadTo(_charInfo.AdditionalVoiceInfo.Gender, _stream);
	ReadTo(_charInfo.AdditionalVoiceInfo.Age, _stream);
	_charInfo.AdditionalVoiceInfo.Style = ReadString(_stream);
}

void AgentFile::ReadBalloonInfo()
{
	if (!(_charInfo.Flags & CharacterFlags::BalloonEnabled))
		return;

	ReadTo(_charInfo.BalloonInfo.TextLines, _stream);
	ReadTo(_charInfo.BalloonInfo.CharsPerLine, _stream);

	ReadTo(_charInfo.BalloonInfo.ForegroundColor.Red, _stream);
	ReadTo(_charInfo.BalloonInfo.ForegroundColor.Green, _stream);
	ReadTo(_charInfo.BalloonInfo.ForegroundColor.Blue, _stream);
	ReadTo(_charInfo.BalloonInfo.ForegroundColor.Reserved, _stream);

	ReadTo(_charInfo.BalloonInfo.BackgroundColor.Red, _stream);
	ReadTo(_charInfo.BalloonInfo.BackgroundColor.Green, _stream);
	ReadTo(_charInfo.BalloonInfo.BackgroundColor.Blue, _stream);
	ReadTo(_charInfo.BalloonInfo.BackgroundColor.Reserved, _stream);

	ReadTo(_charInfo.BalloonInfo.BorderColor.Red, _stream);
	ReadTo(_charInfo.BalloonInfo.BorderColor.Green, _stream);
	ReadTo(_charInfo.BalloonInfo.BorderColor.Blue, _stream);
	ReadTo(_charInfo.BalloonInfo.BorderColor.Reserved, _stream);

	_charInfo.BalloonInfo.FontName = ReadString(_stream);
	ReadTo(_charInfo.BalloonInfo.FontHeight, _stream);
	ReadTo(_charInfo.BalloonInfo.FontWeight, _stream);
	ReadTo(_charInfo.BalloonInfo.Italic, _stream);
	ReadTo(_charInfo.BalloonInfo.Unknown, _stream);
}

LocalizedInfo AgentFile::GetLocalizedInfo(ushort langId)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	if (_localizationInfo.find(langId) == _localizationInfo.end())
		return {};

	return _localizationInfo[langId];
}

IconImage AgentFile::ReadIconImage() // leitura obrigatória: https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513
{
	IconImage iconImage = {};

	ReadTo(iconImage.IconHeader, _stream);
	iconImage.IconHeader.ColorUsed = 1 << (iconImage.IconHeader.BitsPerPixel * iconImage.IconHeader.Planes);

	size_t colorTableSize = iconImage.IconHeader.ColorUsed * sizeof(RGBQuad);
	size_t pixelDataSize = iconImage.IconHeader.SizeOfImageData;

	iconImage.ColorTable = std::shared_ptr<RGBQuad>((RGBQuad*)malloc(colorTableSize), free);
	iconImage.PixelData = std::shared_ptr<byte>((byte*)malloc(pixelDataSize), free);

	_stream.read((char*)iconImage.ColorTable.get(), colorTableSize);
	_stream.read((char*)iconImage.PixelData.get(), pixelDataSize);

	return iconImage;
}

void AgentFile::ReadAnimationPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);
	std::vector<AnimationPointer> animations = ReadVector<uint, AnimationPointer>(_stream, [](std::ifstream& str) -> AnimationPointer {
		AnimationPointer anim = {};
		anim.AnimationName = ReadString(str);
		ReadTo(anim.InfoLocation, str);

		NormalizeString(anim.AnimationName);
		return anim;
	});

	for (auto& animPointer : animations)
		_animations.insert({ animPointer.AnimationName, animPointer });
}

AnimationInfo AgentFile::GetAnimationInfo(string name)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);

	if (_animations.count(name) == 0)
		return {};

	AnimationPointer* pointer = &_animations[name];
	ACSLocator* pos = &pointer->InfoLocation;

	JumpTo(pos->Offset, _stream);

	AnimationInfo info = {};

	info.AnimationName = ReadString(_stream);
	ReadTo(info.Transition, _stream);
	info.ReturnAnimation = ReadString(_stream);
	info.Frames = ReadVector<ushort, FrameInfo>(_stream, [](std::ifstream& str) -> FrameInfo {
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
				int regionSize = 0;
				ReadTo(regionSize, str);
				Skip(regionSize, str);
			}

			return oi;
		});

		return fi;
	});

	return info;
}

void AgentFile::ReadImagePointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);

	_imagePointers = ReadVector<uint, ImagePointer>(_stream, NULL);
}

ImageData AgentFile::ReadImageData(uint index)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= _imagePointers.size())
		throw std::invalid_argument("Imagem inexistente.");

	ImagePointer imgPointer = _imagePointers[index];
	ImageInfo imgInfo = {};

	JumpTo(imgPointer.LocationOfImage.Offset, _stream);

	ReadTo(imgInfo.Unknown, _stream);
	ReadTo(imgInfo.Width, _stream);
	ReadTo(imgInfo.Height, _stream);
	ReadTo(imgInfo.Compressed, _stream);
	ReadTo(imgInfo.ImageData.SizeOfData, _stream);

	byte* imgData = (byte*)malloc(imgInfo.ImageData.SizeOfData);

	if (!imgData)
		throw std::runtime_error("Falha ao alocar memória.");

	_stream.read((char*)imgData, imgInfo.ImageData.SizeOfData);

	size_t uncompressedImageSize = (size_t)imgInfo.Width * (size_t)imgInfo.Height; // sempre 8bpp
	byte* uncompressedImage = (byte*)malloc(uncompressedImageSize);

	if (!uncompressedImage)
		throw std::runtime_error("Falha ao alocar memória.");

	DecompressData(imgData, imgInfo.ImageData.SizeOfData, uncompressedImage);
	free(imgData);

	return { 
		imgInfo.Width, 
		imgInfo.Height, 
		std::shared_ptr<byte>(uncompressedImage, free), 
		uncompressedImageSize 
	};
}

RgnData AgentFile::ReadImageRegion(unsigned int index)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= _imagePointers.size())
		throw std::runtime_error("Imagem inexistente.");

	ImagePointer imgPointer = _imagePointers[index];
	ImageInfo imgInfo = {};

	JumpTo(imgPointer.LocationOfImage.Offset, _stream);

	ReadTo(imgInfo.Unknown, _stream);
	ReadTo(imgInfo.Width, _stream);
	ReadTo(imgInfo.Height, _stream);
	ReadTo(imgInfo.Compressed, _stream);
	ReadTo(imgInfo.ImageData.SizeOfData, _stream);

	Skip(imgInfo.ImageData.SizeOfData, _stream);

	CompressedData cd = {};

	ReadTo(cd.CompressedSize, _stream);
	ReadTo(cd.OriginalSize, _stream);

	int rawSize = cd.CompressedSize ? cd.CompressedSize : cd.OriginalSize;

	byte* rgnData = (byte*)malloc(rawSize);

	if (!rgnData)
		throw std::runtime_error("Falha ao alocar memória.");

	_stream.read((char*)rgnData, rawSize);

	cd.Data = std::shared_ptr<byte>(rgnData, free);

	return ReadRegionData(&cd);
}

AudioData AgentFile::ReadAudioData(uint index)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= _audioPointers.size())
		throw std::invalid_argument("Áudio inexistente.");

	AudioPointer ap = _audioPointers[index];

	JumpTo(ap.AudioData.Offset, _stream);

	byte* audioData = (byte*)calloc(ap.AudioData.Size, 1);

	_stream.read((char*)audioData, ap.AudioData.Size);

	return { std::shared_ptr<byte>(audioData, free), ap.AudioData.Size };
}

std::vector<string> AgentFile::GetAnimationsList()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<string> names = {};

	for (const auto& animInfo : _animations)
		names.push_back(animInfo.first);

	return names;
}

void AgentFile::ReadAudioPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);
	_audioPointers = ReadVector<uint, AudioPointer>(_stream, NULL);
}

RgnData AgentFile::ReadRegionData(CompressedData* cd)
{
	RgnData out = {};

	std::shared_ptr<byte> outputBuffer = cd->Data;

	if (cd->CompressedSize != 0)
	{
		outputBuffer = std::shared_ptr<byte>((byte*)malloc(cd->OriginalSize), free);
		DecompressData(cd->Data.get(), cd->CompressedSize, outputBuffer.get());
	}

	if (!outputBuffer || cd->OriginalSize < 32)
		throw std::runtime_error("Falha ao alocar ou ler memória.");

	memcpy(&out.Header, outputBuffer.get(), sizeof(out.Header));

	// os outros rects não têm tanto uso...

	return out;
}

// Essa função espera que o buffer de saída tenha o tamanho correto
void AgentFile::DecompressData(byte* inputBuffer, size_t inputSize, byte* outputBuffer)
{
	BitReader br = BitReader(inputBuffer, inputSize);

	constexpr byte bitCountTable[] = {
		6, 9, 12, 20
	};

	constexpr ushort valueSumTable[] = {
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

std::vector<std::wstring> AgentFile::GetAvailableStates()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<string> names = {};

	for (const auto& stateInfo : _animationStates)
		names.push_back(stateInfo.first);

	return names;
}

TrayIcon AgentFile::GetAgentIcon()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	return _agentIcon;
}

StateInfo AgentFile::GetStateInfo(string name)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);
	if (!_animationStates.count(name))
		return {};

	return _animationStates[name];
}
