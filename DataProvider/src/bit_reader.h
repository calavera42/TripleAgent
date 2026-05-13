#pragma once
#include <fstream>

class BitReader {
public:
	int m_curBit = 8;
	uint8_t m_curByte = 0;
	const uint8_t* m_position = nullptr;
	size_t m_length = 0;
	size_t m_bytesRead = 0;

	BitReader(const uint8_t* pos, size_t inputSize)
	{
		m_position = pos;
		m_length = inputSize;
	}

	uint8_t ReadBit() {
		if (m_curBit == 8) 
		{
			if (m_bytesRead == m_length)
				throw ("Tentativa de ler fora do buffer.");

			m_curByte = *m_position;
			m_curBit = 0;

			m_position++;
			m_bytesRead++;
		}
		uint8_t value = (uint8_t)((m_curByte >> m_curBit) & 1);

		m_curBit++;
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