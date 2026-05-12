#pragma once
#include "Types.h"
#include <fstream>

class BitReader {
public:
	int CurBit = 8;
	uint8_t CurByte = 0;
	uint8_t* Position = nullptr;
	size_t Length = 0;
	size_t BytesRead = 0;

	BitReader(uint8_t* pos, size_t inputSize)
	{
		Position = pos;
		Length = inputSize;
	}

	uint8_t ReadBit() {
		if (CurBit == 8) 
		{
			if (BytesRead == Length)
				throw ("Tentativa de ler fora do buffer.");

			CurByte = *Position;
			CurBit = 0;

			Position++;
			BytesRead++;
		}
		uint8_t value = (uint8_t)((CurByte >> CurBit) & 1);

		CurBit++;
		return value;
	}
	
	uint32_t ReadBits(int bitCount) 
	{
		uint32_t output = 0;

		for (int i = 0; i < bitCount; i++)
			output |= (uint32_t)(ReadBit() << i);

		return output;
	}

	uint8_t CountSequentialBits(int maxBits) 
	{
		uint8_t seq = 0;
		for (int i = 0; i < maxBits; i++)
		{
			if (ReadBit() == 0)
				break;
			seq++;
		}
		return seq;
	}
};