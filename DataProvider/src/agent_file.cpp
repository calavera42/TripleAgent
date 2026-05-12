#include "agent_file.h"

wstring AgentFile::ReadString(std::ifstream& str)
{
	int length = 0;
	wchar_t curChar = 0;
	wstring output = L"";

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

LoadResult AgentFile::Load(std::string path)
{
	_stream = std::ifstream(path, std::ios_base::binary | std::ios_base::in);

	if (!_stream.is_open())
		return LoadResult::FailToOpenStream;

	ReadTo(FileHeader, _stream);

	if (FileHeader.Signature != ACS_MAGIC)
		return LoadResult::InvalidSignature;

	ReadCharInfo(&FileHeader.CharacterInfo);

	if (_charInfo.MajorVersion != ACS_MAJOR_VERSION)
		return LoadResult::IncompatibleVersion;

	ReadAnimationPointers(&FileHeader.AnimationInfo);
	ReadImagePointers(&FileHeader.ImageInfo);
	ReadAudioPointers(&FileHeader.AudioData);

	_initialized = true;

	return LoadResult::Success;
}

CharacterInfo AgentFile::GetCharacterInfo()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	return _charInfo;
}

void AgentFile::NormalizeString(wstring& s)
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

	_charInfo.ColorTable = ReadVector<uint32_t, RGBQuad>(_stream, NULL);

	ReadTo(_charInfo.HasTrayIcon, _stream);

	if (_charInfo.HasTrayIcon)
	{
		ReadTo(_agentIcon.SizeOfMaskData, _stream);
		_agentIcon.MaskBitmapData = ReadIconImage();

		ReadTo(_agentIcon.SizeOfColorData, _stream);
		_agentIcon.ColorBitmapData = ReadIconImage();
	}

	std::vector<StateInfo> states = ReadVector<uint16_t, StateInfo>(_stream, [](std::ifstream& str) -> StateInfo {
		StateInfo temp;

		temp.StateName = ReadString(str);
		temp.Animations = ReadVector<uint16_t, wstring>(str, ReadString);

		NormalizeString(temp.StateName);

		return temp;
	});

	for (auto& curState : states)
		_animationStates.insert({ curState.StateName, curState });

	JumpTo(localizedInfo.Offset, _stream);
	std::vector<LocalizedInfo> locInfo = ReadVector<uint16_t, LocalizedInfo>(_stream, [](std::ifstream& str) -> LocalizedInfo {
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

LocalizedInfo AgentFile::GetLocalizedInfo(LangId langId)
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
	iconImage.PixelData = std::shared_ptr<uint8_t>((uint8_t*)malloc(pixelDataSize), free);

	_stream.read((char*)iconImage.ColorTable.get(), colorTableSize);
	_stream.read((char*)iconImage.PixelData.get(), pixelDataSize);

	return iconImage;
}

void AgentFile::ReadAnimationPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);
	std::vector<AnimationPointer> animations = ReadVector<uint32_t, AnimationPointer>(_stream, [](std::ifstream& str) -> AnimationPointer {
		AnimationPointer anim = {};
		anim.AnimationName = ReadString(str);
		ReadTo(anim.InfoLocation, str);

		NormalizeString(anim.AnimationName);
		return anim;
	});

	for (auto& animPointer : animations)
		_animations.insert({ animPointer.AnimationName, animPointer });
}

AnimationInfo AgentFile::GetAnimationInfo(wstring name)
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
	info.Frames = ReadVector<uint16_t, FrameInfo>(_stream, [](std::ifstream& str) -> FrameInfo {
		FrameInfo fi = {};

		fi.Images = ReadVector<uint16_t, FrameImage>(str, NULL);

		ReadTo(fi.AudioIndex, str);
		ReadTo(fi.FrameDuration, str);
		ReadTo(fi.ExitFrameIndex, str);

		fi.Branches = ReadVector<uint8_t, BranchInfo>(str, NULL);

		fi.Overlays = ReadVector<uint8_t, OverlayInfo>(str, [](std::ifstream& str) -> OverlayInfo {
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

	_imagePointers = ReadVector<uint32_t, ImagePointer>(_stream, NULL);
}

ImageData AgentFile::ReadImageData(uint32_t index)
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

	std::vector<uint8_t> imgData = {};
	std::vector<uint8_t> uncompressedData = {};

	imgData.resize(imgInfo.ImageData.SizeOfData);
	uncompressedData.resize(imgInfo.Width * imgInfo.Height); // sempre 8bpp

	_stream.read(reinterpret_cast<char*>(imgData.data()), imgInfo.ImageData.SizeOfData);

	auto data = DecompressData(imgData, uncompressedData);

	return { 
		imgInfo.Width, 
		imgInfo.Height, 
		uncompressedData
	};
}

RgnData AgentFile::ReadImageRegion(uint32_t index)
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

	uint8_t* rgnData = (uint8_t*)malloc(rawSize);

	if (!rgnData)
		throw std::runtime_error("Falha ao alocar memória.");

	_stream.read((char*)rgnData, rawSize);

	cd.Data = std::shared_ptr<uint8_t>(rgnData, free);

	return ReadRegionData(&cd);
}

std::span<uint8_t> AgentFile::ReadAudioData(uint32_t index)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= _audioPointers.size())
		throw std::invalid_argument("Áudio inexistente.");

	AudioPointer ap = _audioPointers[index];

	JumpTo(ap.AudioData.Offset, _stream);

	std::vector<uint8_t> data = {};
	data.resize(ap.AudioData.Size);

	_stream.read(reinterpret_cast<char*>(data.data()), ap.AudioData.Size);
	return data;
}

std::vector<wstring> AgentFile::GetAnimationsList()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<wstring> names = {};

	for (const auto& animInfo : _animations)
		names.push_back(animInfo.first);

	return names;
}

void AgentFile::ReadAudioPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, _stream);
	_audioPointers = ReadVector<uint32_t, AudioPointer>(_stream, NULL);
}

// Essa função espera que o buffer de saída tenha o tamanho correto
bool AgentFile::DecompressData(const std::vector<uint8_t>& inputBuffer, std::vector<uint8_t>& output)
{
	BitReader br = BitReader(inputBuffer.data(), inputBuffer.size());

	constexpr uint8_t bitCountTable[] = {
		6, 9, 12, 20
	};

	constexpr uint16_t valueSumTable[] = {
		1, 65, 577, 4673
	};

	uint32_t index = 0;

	br.ReadBits(8); // padding rsrs

	while (true) 
	{
		if (!br.ReadBit()) // byte normal
		{ 
			output[index] = (uint8_t)br.ReadBits(8);
			index++;
			continue;
		}

		uint16_t countOfBytesToDecode = 2; // pelo menos 2 bytes codificados
		uint8_t offsetSequentialBits = br.CountSequentialBits(3);
		uint8_t offsetBitCount = bitCountTable[offsetSequentialBits];
		uint32_t offset = br.ReadBits(offsetBitCount);

		if (offsetBitCount == 20)
		{
			if (offset == 0x000FFFFF)
				break; // fim dos dados
			else
				countOfBytesToDecode++;
		}

		offset += valueSumTable[offsetSequentialBits];

		// a quantidade de bits sequenciais dita quantos bytes serão decodificados
		uint8_t decBytesSequentialBits = br.CountSequentialBits(11); // o máximo de bits é 11

		if (decBytesSequentialBits == 11 && br.ReadBit()) // um bit extra que sempre deve ser 0
			throw ("O 12º bit da sequência é 1.");

		if (decBytesSequentialBits)
		{
			countOfBytesToDecode += (uint16_t)((1 << decBytesSequentialBits) - 1);
			countOfBytesToDecode += (uint16_t)br.ReadBits(decBytesSequentialBits);
		}

		for (int i = 0; i < countOfBytesToDecode; i++) // copiar os dados para o buffer de saída
		{
			output[index] = output[index - offset];
			index++;
		}
	}
}

std::vector<std::wstring> AgentFile::GetAvailableStates()
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<wstring> names = {};

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

StateInfo AgentFile::GetStateInfo(wstring name)
{
	if (!_initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);
	if (!_animationStates.count(name))
		return {};

	return _animationStates[name];
}
