#pragma once

#include <inttypes.h>

#include <string>
#include <vector>

#include <map>

struct Token 
{
	enum Type : uint8_t
	{
		Unknown,

		// texto normal
		StringLiteral,

		// títulos
		H1, // #
		H2, // ##
		H3, // ###
		H4, // ####
		H5, // #####
		H6, // ######

		PreH1, // ======
		PreH2, // ------

		// caracteres invisíveis
		LineBreak, // \n
		Whitespace, // (espaço) e \t

		// estilo
		Bold, // **
		Italic, // *

		OrderedListIdentifier, // n.
		UnorderedListIdentifier, // -

		ThematicBreak // famígero <hr> no html
	};

	Type TokenType;

	int Identation; // em espaços (\t = 2 espaços)
	std::wstring Data;

	void ToString() const // só pro debug
	{
		std::map<Type, const char*> lut = {
			{ Unknown, "Unknown" },
			{ StringLiteral, "StringLiteral"},

			{ H1, "H1"},
			{ H2, "H2"},
			{ H3, "H3"},
			{ H4, "H4"},
			{ H5, "H5"},
			{ H6, "H6"},

			{ PreH1, "PreH1"},
			{ PreH2, "PreH2"},

			{ LineBreak, "LineBreak"},
			{ Whitespace, "Whitespace"},

			{ Bold, "Bold"},
			{ Italic, "Italic"},

			{ OrderedListIdentifier, "OrderedListIdentifier"},
			{ UnorderedListIdentifier, "UnurderedListIdentifier"},

			{ ThematicBreak, "ThematicBreak"},
		};

		printf("[%s,\t%d] = %ls", lut[TokenType], Identation, Data.c_str());
	}
};

class Lexer
{
public:

	std::vector<Token> ParseString(std::wstring s)  // onelonecoder my beloved
	{
		enum TokeniserState : uint8_t
		{
			NewToken,
			
			EndToken
		};

		std::vector<Token> output{};

		TokeniserState stateNow = NewToken;
		TokeniserState stateNext = NewToken;

		std::wstring tab{};
		Token curToken{};

		auto charNow = s.begin();
		while (charNow != s.end()) 
		{
			switch (stateNow) 
			{
				case NewToken:
				{
					tab.clear();
					curToken = { Token::Unknown, 0, L""};

					if (std::isdigit(charNow[0]) && charNow[1] == '.')
					{
						
						break;
					}


				}
				break;
				case EndToken:
				{
					
				}
				break;
			}

			stateNow = stateNext;
		}

		return output;
	}
};