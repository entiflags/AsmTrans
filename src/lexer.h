#pragma once
#include <string>
#include <vector>
#include <string_view>

enum class token_type
{
	identifier,
	number,
	assign,
	plus,
	minus,
	mul,
	div,
	colon,
	newline,
	indent,
	detent,
	equal,
	notequal,
	less,
	greater,
	eof
};

struct token
{
	token_type type;
	std::string value;
};

class lexer
{
public:
	explicit lexer(std::string_view source) : src(source), cursor(0) {
		indent_stack.push_back(0);
	}
	std::vector<token> tokenize();
private:
	std::string_view src;
	size_t cursor;
	std::vector<int> indent_stack;

	char peek() const
	{
		return cursor < src.size() ? src[cursor] : '\0';
	}
	char advance()
	{
		return cursor < src.size() ? src[cursor++] : '\0';
	}
};