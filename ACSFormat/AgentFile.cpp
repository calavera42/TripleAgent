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

void AgentFile::Load(std::string path)
{
	Stream = std::ifstream();

	Stream.open(path, std::ios_base::binary | std::ios_base::in);

	if (!Stream.is_open()) {
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Triple Agent", "Falha ao abrir arquivo.", NULL);
		return;
	}

	ReadTo(FileHeader, Stream);

	if (FileHeader.Signature != 0xABCDABC3) {
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Triple Agent", "A assinatura do arquivo fornecido é inválida.", NULL);
		return; // TODO: código de erro
	}

	ReadCharInfo(&FileHeader.CharacterInfo);

	ReadAnimationInfo(&FileHeader.AnimationInfo);
	ReadImageInfo(&FileHeader.ImageInfo);
	ReadAudioInfo(&FileHeader.AudioInfo);
}

void AgentFile::ReadCharInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);

	ACSLocator localizedInfo = {};

	ReadTo(CharInfo.MinorVersion, Stream);
	ReadTo(CharInfo.MajorVersion, Stream);
	ReadTo(localizedInfo, Stream);
	ReadTo(CharInfo.CharId, Stream);
	ReadTo(CharInfo.Width, Stream);
	ReadTo(CharInfo.Height, Stream);
	ReadTo(CharInfo.TransparentColorIndex, Stream);
	ReadTo(CharInfo.Flags, Stream);
	ReadTo(CharInfo.AnimationMajorVersion, Stream);
	ReadTo(CharInfo.AnimationMinorVersion, Stream);

	ReadVoiceInfo();
	ReadBalloonInfo();

	CharInfo.Palette = ReadVector<uint, RGBQuad>(Stream, NULL);

	CharacterPalette.ncolors = (int)CharInfo.Palette.size();
	CharacterPalette.colors = (SDL_Color*)calloc(CharInfo.Palette.size(), sizeof(RGBQuad));

	for (int i = 0; i < CharInfo.Palette.size(); i++)
	{
		RGBQuad rgb = CharInfo.Palette[i];

		CharacterPalette.colors[i] = SDL_Color{ rgb.Red, rgb.Green, rgb.Blue, 255 };
	}

	ReadTo(CharInfo.TrayIconEnabled, Stream);

	if (CharInfo.TrayIconEnabled)
		AgentTrayIcon = ReadTrayIcon();

	std::vector<StateInfo> states = ReadVector<ushort, StateInfo>(Stream, [](std::ifstream& str) -> StateInfo {
		StateInfo temp;

		temp.StateName = ReadString(str);
		temp.Animations = ReadVector<ushort, string>(str, ReadString);

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
	if (!((uint)CharInfo.Flags & (uint)CharacterFlags::VoiceOutput))
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

LocalizedInfo* AgentFile::GetLocalizedInfo(ushort langId)
{
	return &LocalizationInfo[langId];
}

SDL_Surface* AgentFile::ReadTrayIcon()
{
	TrayIcon output = {};

	ReadTo(output.SizeOfMonochromeData, Stream);
	output.MonochromeBitmapData = ReadIconImage();
	ReadTo(output.SizeOfColorData, Stream);
	output.ColorBitmapData = ReadIconImage();

	SDL_Surface* image = RasterizeIconImage(output.ColorBitmapData);
	SDL_Surface* mask = RasterizeIconImage(output.MonochromeBitmapData);

	int width = output.ColorBitmapData.IconHeader.Width;
	int height = output.ColorBitmapData.IconHeader.Height;

	SDL_Surface* icon = SDL_CreateRGBSurfaceWithFormat(
		0, 
		width, 
		height, 
		32, 
		SDL_PIXELFORMAT_RGBA4444
	);
	SDL_Renderer* rend = SDL_CreateSoftwareRenderer(icon);

	SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
	SDL_RenderClear(rend);

	for(int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) 
		{
			SDL_Color* iconPixel = (SDL_Color*)image->pixels + ((width * y) + x);
			SDL_Color* maskPixel = (SDL_Color*)mask->pixels + ((width * y) + x);

			if (maskPixel->r == 255)
				continue;

			SDL_SetRenderDrawColor(rend, iconPixel->r, iconPixel->g, iconPixel->b, 255);
			SDL_RenderDrawPoint(rend, x, width - y);
		}

	SDL_FreeSurface(image);
	SDL_FreeSurface(mask);

	SDL_DestroyRenderer(rend);

	return icon;
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

SDL_Surface* AgentFile::RasterizeIconImage(IconImage& im)
{
	SDL_PixelType lookup[9] = {
		SDL_PIXELTYPE_UNKNOWN,
		SDL_PIXELTYPE_INDEX1,
		SDL_PIXELTYPE_INDEX2,
		SDL_PIXELTYPE_UNKNOWN,
		SDL_PIXELTYPE_INDEX4,
		SDL_PIXELTYPE_UNKNOWN,
		SDL_PIXELTYPE_UNKNOWN,
		SDL_PIXELTYPE_UNKNOWN,
		SDL_PIXELTYPE_INDEX8
	};

	SDL_Surface* icon = SDL_CreateRGBSurfaceWithFormat(
		0,
		im.IconHeader.Width,
		im.IconHeader.Height,
		32,
		SDL_DEFINE_PIXELFORMAT(
			lookup[im.IconHeader.BitsPerPixel],
			SDL_BITMAPORDER_4321,
			0,
			im.IconHeader.BitsPerPixel,
			0
		)
	);

	SDL_Palette* palette = SDL_AllocPalette(im.IconHeader.ColorUsed);

	for (int i = 0; i < palette->ncolors; i++) {
		RGBQuad col = im.ColorTable[i];
		SDL_Color sCol = { col.Red, col.Green, col.Blue, 255 };

		SDL_SetPaletteColors(palette, &sCol, i, 1);
	}

	memcpy(icon->pixels, im.Data, im.IconHeader.SizeOfImageData);

	SDL_SetSurfacePalette(icon, palette);

	free(im.Data);
	free(im.ColorTable);
	SDL_FreePalette(palette);

	return SDL_ConvertSurfaceFormat(icon, SDL_PIXELFORMAT_RGBA32, 0); // tem q ser
}

void AgentFile::ReadAnimationInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);
	std::vector<AnimationPointer> animations = ReadVector<uint, AnimationPointer>(Stream, [](std::ifstream& str) -> AnimationPointer {
		AnimationPointer anim = {};
		anim.AnimationName = ReadString(str);
		ReadTo(anim.InfoLocation, str);
		return anim;
	});

	for (auto& animPointer : animations)
		Animations.insert({ animPointer.AnimationName, animPointer });
}

AnimationInfo AgentFile::ReadAnimation(string name)
{
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

void AgentFile::ReadImageInfo(ACSLocator* pos)
{
	JumpTo(pos->Offset, Stream);

	ImagePointers = ReadVector<uint, ImagePointer>(Stream, ReadSimple<ImagePointer>);
}

// É responsabilidade da função chamadora o descarte correto dessa surface.
SDL_Surface* AgentFile::ReadImage(uint index)
{
	if (index >= ImagePointers.size())
		return nullptr;

	ImagePointer* imgPointer = &ImagePointers[index];
	ImageInfo imgInfo = {};

	JumpTo(imgPointer->LocationOfImage.Offset, Stream);
	ReadTo(imgInfo.Unknown, Stream);
	ReadTo(imgInfo.Width, Stream);
	ReadTo(imgInfo.Height, Stream);
	ReadTo(imgInfo.Compressed, Stream);
	ReadTo(imgInfo.ImageData.SizeOfData, Stream);

	imgInfo.ImageData.Data = (byte*)calloc(imgInfo.ImageData.SizeOfData, 1);

	if (imgInfo.ImageData.Data == 0)
		return nullptr;

	Stream.read((char*)imgInfo.ImageData.Data, imgInfo.ImageData.SizeOfData);

	size_t uncompressedImageSize = (size_t)imgInfo.Width * (size_t)imgInfo.Height;
	byte* uncompressedImage = (byte*)calloc(uncompressedImageSize, 1);

	if (uncompressedImage == 0)
		return nullptr;

	DecompressData(imgInfo.ImageData.Data, imgInfo.ImageData.SizeOfData, uncompressedImage);
	free(imgInfo.ImageData.Data);

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, imgInfo.Width, imgInfo.Height, 8, SDL_PIXELFORMAT_INDEX8);

	std::memcpy(surface->pixels, uncompressedImage, uncompressedImageSize);

	free(uncompressedImage);

	SDL_SetSurfacePalette(surface, &CharacterPalette);

	RGBQuad colorKey = CharInfo.Palette[CharInfo.TransparentColorIndex];
	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, colorKey.Red, colorKey.Green, colorKey.Blue));

	return surface;
}

AudioInfo AgentFile::ReadAudio(uint index)
{
	AudioPointer ap = AudioPointers[index];

	JumpTo(ap.AudioData.Offset, Stream);

	void* audioData = (void*)calloc(ap.AudioData.Size, 1);

	Stream.read((char*)audioData, ap.AudioData.Size);

	return { audioData, ap.AudioData.Size };
}

void AgentFile::ReadAudioInfo(ACSLocator* pos)
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

	br.ReadBits(8);

	while (true) 
	{
		if (!br.ReadBit()) { // byte normal
			outputBuffer[index] = (byte)br.ReadBits(8);
			index++;
			continue;
		}

		ushort countOfBytesToDecode = 2;
		byte offsetSequentialBits = br.CountSequentialBits(3);
		byte offsetBitCount = bitCountTable[offsetSequentialBits];
		uint offset = br.ReadBits(offsetBitCount);

		if (offsetSequentialBits == 3) {
			if (offset == 0x000FFFFF)
				break;
			else
				countOfBytesToDecode++;
		}

		offset += valueSumTable[offsetSequentialBits];
		byte decBytesSequentialBits = br.CountSequentialBits(11);

		if (decBytesSequentialBits == 11 && br.ReadBit())
			throw ("O 12º bit da sequência é 1.");

		if (decBytesSequentialBits)
		{
			countOfBytesToDecode += (ushort)((1 << decBytesSequentialBits) - 1);
			countOfBytesToDecode += (ushort)br.ReadBits(decBytesSequentialBits);
		}

		for (int i = 0; i < countOfBytesToDecode; i++)
		{
			outputBuffer[index] = outputBuffer[index - offset];
			index++;
		}
	}
}

StateInfo* AgentFile::ReadState(string name)
{
	if (!AnimationStates.count(name))
		return nullptr;

	return &AnimationStates[name];
}
