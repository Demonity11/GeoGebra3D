#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <vector>
#include <string_view>
#include <string>

struct Token
{
	enum Type
	{
		Identifier,
		LParen,
		RParen,
		Number,
		Comma
	};

	Type type{};
	std::string_view lexeme{};
};

namespace Lexer
{
	extern std::vector<Token> tokens;
}

void tokenizer(const std::string& input);
std::string_view convertTokenToString_View(Token::Type type);
void printTokens(const std::vector<Token>& tokens);

#endif