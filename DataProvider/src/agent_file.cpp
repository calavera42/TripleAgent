#include "agent_file.h"

std::wstring AgentFile::ReadString(std::ifstream& str)
{
	int length = 0;
	wchar_t curChar = 0;
	std::wstring output = L"";

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

template<typename Type>
Type ReadSimple(std::ifstream& ifs)
{
	Type t{};
	ReadTo(t, ifs);
	return t;
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

LoadResult AgentFile::Load(std::string path)
{
	m_stream = std::ifstream(path, std::ios_base::binary | std::ios_base::in);

	if (!m_stream.is_open())
		return LoadResult::FailToOpenStream;

	ReadTo(m_fileHeader, m_stream);

	if (m_fileHeader.Signature != ACS_MAGIC)
		return LoadResult::InvalidSignature;

	ReadCharInfo(&m_fileHeader.CharacterInfo);

	if (m_charInfo.MajorVersion != ACS_MAJOR_VERSION || 
		m_charInfo.MinorVersion > ACS_MAJOR_VERSION)
		return LoadResult::IncompatibleVersion;

	ReadAnimationPointers(&m_fileHeader.AnimationInfo);
	ReadImagePointers(&m_fileHeader.ImageInfo);
	ReadAudioPointers(&m_fileHeader.AudioData);

	m_initialized = true;

	return LoadResult::Success;
}

CharacterInfo AgentFile::GetCharacterInfo()
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	return m_charInfo;
}

void AgentFile::NormalizeString(std::wstring& s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
}

void AgentFile::ReadCharInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, m_stream);

	ACSLocator localizedInfo = {};

	ReadTo(m_charInfo.MinorVersion, m_stream);
	ReadTo(m_charInfo.MajorVersion, m_stream);
	ReadTo(localizedInfo, m_stream);
	ReadTo(m_charInfo.CharId, m_stream);
	ReadTo(m_charInfo.Width, m_stream);
	ReadTo(m_charInfo.Height, m_stream);
	ReadTo(m_charInfo.TransparencyIndex, m_stream);
	ReadTo(m_charInfo.Flags, m_stream);
	ReadTo(m_charInfo.AnimationMajorVersion, m_stream);
	ReadTo(m_charInfo.AnimationMinorVersion, m_stream);

	ReadVoiceInfo();
	ReadBalloonInfo();

	m_charInfo.ColorTable = ReadVector<uint32_t, RGBQuad>(m_stream, NULL);

	ReadTo(m_charInfo.HasTrayIcon, m_stream);

	if (m_charInfo.HasTrayIcon)
	{
		ReadTo(m_agentIcon.SizeOfMaskData, m_stream);
		m_agentIcon.MaskBitmapData = ReadIconImage();

		ReadTo(m_agentIcon.SizeOfColorData, m_stream);
		m_agentIcon.ColorBitmapData = ReadIconImage();
	}

	std::vector<StateInfo> states = ReadVector<uint16_t, StateInfo>(m_stream, [](std::ifstream& str) -> StateInfo {
		StateInfo temp;

		temp.StateName = ReadString(str);
		temp.Animations = ReadVector<uint16_t, std::wstring>(str, ReadString);

		NormalizeString(temp.StateName);

		return temp;
	});

	for (auto& curState : states)
		m_animationStates.insert({ curState.StateName, curState });

	JumpTo(localizedInfo.Offset, m_stream);
	std::vector<LocalizedInfo> locInfo = ReadVector<uint16_t, LocalizedInfo>(m_stream, [](std::ifstream& str) -> LocalizedInfo {
		LocalizedInfo temp;

		ReadTo(temp.LanguageId, str);
		temp.CharName = ReadString(str);
		temp.CharDescription = ReadString(str);
		temp.CharExtraData = ReadString(str);

		return temp;
	});

	for (auto& loc : locInfo) 
		m_localizationInfo.insert({ loc.LanguageId, loc });
}

void AgentFile::ReadVoiceInfo()
{
	if (!(m_charInfo.Flags & CharacterFlags::VoiceEnabled))
		return;

	ReadTo(m_charInfo.VoiceInfo, m_stream);

	if (!m_charInfo.VoiceInfo.ExtraData)
		return;

	m_charInfo.AdditionalVoiceInfo = {};

	ReadTo(m_charInfo.AdditionalVoiceInfo.LangId, m_stream);
	m_charInfo.AdditionalVoiceInfo.LanguageDialect = ReadString(m_stream);
	ReadTo(m_charInfo.AdditionalVoiceInfo.Gender, m_stream);
	ReadTo(m_charInfo.AdditionalVoiceInfo.Age, m_stream);
	m_charInfo.AdditionalVoiceInfo.Style = ReadString(m_stream);
}

void AgentFile::ReadBalloonInfo()
{
	if (!(m_charInfo.Flags & CharacterFlags::BalloonEnabled))
		return;

	ReadTo(m_charInfo.BalloonInfo.TextLines, m_stream);
	ReadTo(m_charInfo.BalloonInfo.CharsPerLine, m_stream);

	ReadTo(m_charInfo.BalloonInfo.ForegroundColor.Red, m_stream);
	ReadTo(m_charInfo.BalloonInfo.ForegroundColor.Green, m_stream);
	ReadTo(m_charInfo.BalloonInfo.ForegroundColor.Blue, m_stream);
	ReadTo(m_charInfo.BalloonInfo.ForegroundColor.Reserved, m_stream);

	ReadTo(m_charInfo.BalloonInfo.BackgroundColor.Red, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BackgroundColor.Green, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BackgroundColor.Blue, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BackgroundColor.Reserved, m_stream);

	ReadTo(m_charInfo.BalloonInfo.BorderColor.Red, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BorderColor.Green, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BorderColor.Blue, m_stream);
	ReadTo(m_charInfo.BalloonInfo.BorderColor.Reserved, m_stream);

	m_charInfo.BalloonInfo.FontName = ReadString(m_stream);
	ReadTo(m_charInfo.BalloonInfo.FontHeight, m_stream);
	ReadTo(m_charInfo.BalloonInfo.FontWeight, m_stream);
	ReadTo(m_charInfo.BalloonInfo.Italic, m_stream);
	ReadTo(m_charInfo.BalloonInfo.Unknown, m_stream);
}

LocalizedInfo AgentFile::GetLocalizedInfo(LangId langId)
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	if (m_localizationInfo.find(langId) == m_localizationInfo.end())
		return {};

	return m_localizationInfo[langId];
}

IconImage AgentFile::ReadIconImage() // leitura obrigatória: https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513
{
	IconImage iconImage = {};

	ReadTo(iconImage.IconHeader, m_stream);
	iconImage.IconHeader.ColorUsed = 1 << (iconImage.IconHeader.BitsPerPixel * iconImage.IconHeader.Planes);

	size_t colorTableSize = iconImage.IconHeader.ColorUsed * sizeof(RGBQuad);
	size_t pixelDataSize = iconImage.IconHeader.SizeOfImageData;

	iconImage.ColorTable = std::shared_ptr<RGBQuad>((RGBQuad*)malloc(colorTableSize), free);
	iconImage.PixelData = std::shared_ptr<uint8_t>((uint8_t*)malloc(pixelDataSize), free);

	m_stream.read((char*)iconImage.ColorTable.get(), colorTableSize);
	m_stream.read((char*)iconImage.PixelData.get(), pixelDataSize);

	return iconImage;
}

void AgentFile::ReadAnimationPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, m_stream);
	std::vector<AnimationPointer> animations = ReadVector<uint32_t, AnimationPointer>(m_stream, [](std::ifstream& str) -> AnimationPointer {
		AnimationPointer anim = {};
		anim.AnimationName = ReadString(str);
		ReadTo(anim.InfoLocation, str);

		NormalizeString(anim.AnimationName);
		return anim;
	});

	for (auto& animPointer : animations)
		m_animations.insert({ animPointer.AnimationName, animPointer });
}

AnimationInfo AgentFile::GetAnimationInfo(std::wstring name)
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);

	if (m_animations.count(name) == 0)
		return {};

	AnimationPointer* pointer = &m_animations[name];
	ACSLocator* pos = &pointer->InfoLocation;

	JumpTo(pos->Offset, m_stream);

	AnimationInfo info = {};

	info.AnimationName = ReadString(m_stream);
	ReadTo(info.Transition, m_stream);
	info.ReturnAnimation = ReadString(m_stream);
	info.Frames = ReadVector<uint16_t, FrameInfo>(m_stream, [](std::ifstream& str) -> FrameInfo {
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
	JumpTo(pos->Offset, m_stream);

	m_imagePointers = ReadVector<uint32_t, ImagePointer>(m_stream, NULL);
}

ImageData AgentFile::ReadImageData(uint32_t index)
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= m_imagePointers.size())
		throw std::invalid_argument("Imagem inexistente.");

	ImagePointer imgPointer = m_imagePointers[index];
	ImageInfo imgInfo = {};

	JumpTo(imgPointer.LocationOfImage.Offset, m_stream);

	ReadTo(imgInfo.Unknown, m_stream);
	ReadTo(imgInfo.Width, m_stream);
	ReadTo(imgInfo.Height, m_stream);
	ReadTo(imgInfo.Compressed, m_stream);
	ReadTo(imgInfo.ImageData.SizeOfData, m_stream);

	std::vector<uint8_t> imgData = {};
	std::vector<uint8_t> uncompressedData = {};

	imgData.resize(imgInfo.ImageData.SizeOfData);
	uncompressedData.resize(imgInfo.Width * imgInfo.Height); // sempre 8bpp

	m_stream.read(reinterpret_cast<char*>(imgData.data()), imgInfo.ImageData.SizeOfData);

	auto data = DecompressData(imgData, uncompressedData);

	return { 
		imgInfo.Width, 
		imgInfo.Height, 
		uncompressedData
	};
}

std::span<uint8_t> AgentFile::ReadAudioData(uint32_t index)
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	if (index >= m_audioPointers.size())
		throw std::invalid_argument("Áudio inexistente.");

	AudioPointer ap = m_audioPointers[index];

	JumpTo(ap.AudioData.Offset, m_stream);

	std::vector<uint8_t> data = {};
	data.resize(ap.AudioData.Size);

	m_stream.read(reinterpret_cast<char*>(data.data()), ap.AudioData.Size);
	return data;
}

std::vector<std::wstring> AgentFile::GetAnimationsList()
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<std::wstring> names = {};

	for (const auto& animInfo : m_animations)
		names.push_back(animInfo.first);

	return names;
}

void AgentFile::ReadAudioPointers(ACSLocator* pos)
{
	JumpTo(pos->Offset, m_stream);
	m_audioPointers = ReadVector<uint32_t, AudioPointer>(m_stream, NULL);
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
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	std::vector<std::wstring> names = {};

	for (const auto& stateInfo : m_animationStates)
		names.push_back(stateInfo.first);

	return names;
}

TrayIcon AgentFile::GetAgentIcon()
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	return m_agentIcon;
}

StateInfo AgentFile::GetStateInfo(std::wstring name)
{
	if (!m_initialized)
		throw std::runtime_error("Não inicializado.");

	NormalizeString(name);
	if (!m_animationStates.count(name))
		return {};

	return m_animationStates[name];
}
