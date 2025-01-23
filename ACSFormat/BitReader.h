#pragma once
#include "..\types.h"
#include <fstream>

class BitReader {
public:
	int CurBit = 8;
	byte CurByte = 0;
	byte* Position = nullptr;
	size_t Length = 0;
	size_t BytesRead = 0;

	BitReader(byte* pos, size_t inputSize)
	{
		Position = pos;
		Length = inputSize;
	}

	byte PeekBit() 
	{
		int tempBit = CurBit;
		int tempByte = CurByte;
		size_t bytesRead = BytesRead;
		byte* pos = Position;

		byte bit = ReadBit();

		CurBit = tempBit;
		CurByte = tempByte;
		Position = pos;
		BytesRead = bytesRead;

		return bit;
	}

	byte ReadBit() {
		if (CurBit == 8) 
		{
			if (BytesRead == Length)
				throw ("Tentativa de ler fora da do buffer.");

			CurByte = *Position;
			CurBit = 0;

			Position++;
			BytesRead++;
		}
		byte value = (byte)((CurByte >> CurBit) & 1);

		CurBit++;
		return value;
	}
	
	uint ReadBits(int bitCount) 
	{
		uint output = 0;

		for (int i = 0; i < bitCount; i++)
			output |= (uint)(ReadBit() << i);

		return output;
	}

	byte CountSequentialBits(int maxBits) 
	{
		byte seq = 0;
		for (int i = 0; i < maxBits; i++)
		{
			if (ReadBit() == 0)
				break;
			seq++;
		}
		return seq;
	}
};